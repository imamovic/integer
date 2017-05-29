#include "integer64.hpp"

class Integer64 : public node::ObjectWrap {
public:
	
	static void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		v8::HandleScope scope(isolate);
		
		v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		t->SetClassName(StringFromLatin1(isolate, "Integer64"));
		
		NODE_SET_PROTOTYPE_GETTER(t, "low", Low);
		NODE_SET_PROTOTYPE_GETTER(t, "high", High);
		NODE_SET_PROTOTYPE_METHOD(t, "add", Add);
		NODE_SET_PROTOTYPE_METHOD(t, "subtract", Subtract);
		NODE_SET_PROTOTYPE_METHOD(t, "multiply", Multiply);
		NODE_SET_PROTOTYPE_METHOD(t, "divide", Divide);
		NODE_SET_PROTOTYPE_METHOD(t, "modulo", Modulo);
		NODE_SET_PROTOTYPE_METHOD(t, "negate", Negate);
		NODE_SET_PROTOTYPE_METHOD(t, "abs", Abs);
		NODE_SET_PROTOTYPE_METHOD(t, "and", And);
		NODE_SET_PROTOTYPE_METHOD(t, "or", Or);
		NODE_SET_PROTOTYPE_METHOD(t, "xor", Xor);
		NODE_SET_PROTOTYPE_METHOD(t, "not", Not);
		NODE_SET_PROTOTYPE_METHOD(t, "shiftLeft", ShiftLeft);
		NODE_SET_PROTOTYPE_METHOD(t, "shiftRight", ShiftRight);
		NODE_SET_PROTOTYPE_METHOD(t, "equals", Equals);
		NODE_SET_PROTOTYPE_METHOD(t, "notEquals", NotEquals);
		NODE_SET_PROTOTYPE_METHOD(t, "greaterThan", GreaterThan);
		NODE_SET_PROTOTYPE_METHOD(t, "greaterThanOrEquals", GreaterThanOrEquals);
		NODE_SET_PROTOTYPE_METHOD(t, "lessThan", LessThan);
		NODE_SET_PROTOTYPE_METHOD(t, "lessThanOrEquals", LessThanOrEquals);
		NODE_SET_PROTOTYPE_METHOD(t, "compare", Compare);
		NODE_SET_PROTOTYPE_METHOD(t, "bitSizeAbs", BitSizeAbs);
		NODE_SET_PROTOTYPE_METHOD(t, "isEven", IsEven);
		NODE_SET_PROTOTYPE_METHOD(t, "isOdd", IsOdd);
		NODE_SET_PROTOTYPE_METHOD(t, "isPositive", IsPositive);
		NODE_SET_PROTOTYPE_METHOD(t, "isNegative", IsNegative);
		NODE_SET_PROTOTYPE_METHOD(t, "isZero", IsZero);
		NODE_SET_PROTOTYPE_METHOD(t, "isNonZero", IsNonZero);
		NODE_SET_PROTOTYPE_METHOD(t, "isSafe", IsSafe);
		NODE_SET_PROTOTYPE_METHOD(t, "isUnsafe", IsUnsafe);
		NODE_SET_PROTOTYPE_METHOD(t, "toNumberUnsafe", ToNumberUnsafe);
		NODE_SET_PROTOTYPE_METHOD(t, "toString", ToString);
		NODE_SET_PROTOTYPE_METHOD(t, "valueOf", ValueOf);
		
		v8::Local<v8::Function> c = t->GetFunction(isolate->GetCurrentContext()).ToLocalChecked();
		exports->Set(StringFromLatin1(isolate, "Integer64"), c);
		
		NODE_SET_METHOD(v8::Local<v8::Object>::Cast(c), "fromString", FromString);
		NODE_SET_METHOD(v8::Local<v8::Object>::Cast(c), "fromNumber", FromNumber);
		NODE_SET_METHOD(v8::Local<v8::Object>::Cast(c), "fromBits", FromBits);
		NODE_SET_METHOD(v8::Local<v8::Object>::Cast(c), "isInteger64", IsInteger64);
		
		constructor.Reset(isolate, c);
		constructorTemplate.Reset(isolate, t);
		constructing_privileges = false;
		constructing_value = 0;
	}
	
private:
	explicit Integer64(int64_t _value) : node::ObjectWrap(), value(_value) {}
	
	NODE_METHOD(New) {
		if (info.IsConstructCall()) {
			if (!constructing_privileges) {
				return ThrowTypeError(info, "Disabled constructor (use fromString, fromNumber, or fromBits)");
			}
			constructing_privileges = false;
			(new Integer64(constructing_value))->Wrap(info.This());
			return info.GetReturnValue().Set(info.This());
		}
		if (info.Length() == 0) return ReturnNew(info, 0);
		Result cast = Cast(info, info[0]);
		cast.error ? ThrowException(info, *cast.error) : ReturnNew(info, cast.Checked());
	}
	
	NODE_METHOD(FromBits) {
		int32_t low;
		int32_t high = 0;
		REQUIRE_ARGUMENT_INT32(first, low);
		if (info.Length() > 1) {REQUIRE_ARGUMENT_INT32(second, high);}
		ReturnNew(info, (int64_t)((((uint64_t)((uint32_t)high)) << 32) | (uint32_t)low));
	}
	
	NODE_METHOD(FromNumber) {
		REQUIRE_ARGUMENT_NUMBER(first, v8::Local<v8::Number> number);
		Result cast = Cast(number);
		if (!cast.error) return ReturnNew(info, cast.Checked());
		if (info.Length() == 1) return ThrowException(info, *cast.error);
		Result def = Cast(info, info[1]);
		if (!def.error) return ReturnNew(info, def.Checked());
		ThrowTypeError(info, "The default value could not be converted to an Integer64");
	}
	
	NODE_METHOD(FromString) {
		REQUIRE_ARGUMENT_STRING(first, v8::Local<v8::String> string);
		uint32_t radix = 10;
		if (info.Length() > 1) {
			REQUIRE_ARGUMENT_UINT32(second, radix);
			if (radix < 2 || radix > 36) return ThrowRangeError(info, "Radix argument must be within 2 - 36");
		}
		Result cast = Cast(string, (uint8_t)radix);
		if (!cast.error) return ReturnNew(info, cast.Checked());
		if (info.Length() < 3) return ThrowException(info, *cast.error);
		Result def = Cast(info, info[2]);
		if (!def.error) return ReturnNew(info, def.Checked());
		ThrowTypeError(info, "The default value could not be converted to an Integer64");
	}
	
	NODE_METHOD(IsInteger64) {
		Return(info, info.Length() != 0 && HasInstance(info, info[0]));
	}
	
	NODE_GETTER(Low) { UseValue;
		Return(info, (int32_t)((uint32_t)(((uint64_t)value) & U32_in_U64)));
	}
	
	NODE_GETTER(High) { UseValue;
		Return(info, (int32_t)((uint32_t)(((uint64_t)value) >> 32)));
	}
	
	NODE_METHOD(Add) { UseValue; UseArgument;
		if ((arg > 0 && value > MAX_VALUE - arg) || (arg < 0 && value < MIN_VALUE - arg))
			return ThrowRangeError(info, "Integer overflow");
		ReturnNew(info, value + arg);
	}
	
	NODE_METHOD(Subtract) { UseValue; UseArgument;
		if ((arg < 0 && value > MAX_VALUE + arg) || (arg > 0 && value < MIN_VALUE + arg))
			return ThrowRangeError(info, "Integer overflow");
		ReturnNew(info, value - arg);
	}
	
	NODE_METHOD(Multiply) { UseValue; UseArgument;
		if (value > 0) {
			if (arg > 0) {
				if (value > MAX_VALUE / arg) return ThrowRangeError(info, "Integer overflow");
			} else {
				if (arg < MIN_VALUE / value) return ThrowRangeError(info, "Integer overflow");
			}
		} else {
			if (arg > 0) {
				if (value < MIN_VALUE / arg) return ThrowRangeError(info, "Integer overflow");
			} else {
				if (value != 0 && arg < MAX_VALUE / value) return ThrowRangeError(info, "Integer overflow");
			}
		}
		ReturnNew(info, value * arg);
	}
	
	NODE_METHOD(Divide) { UseValue; UseArgument;
		if (arg == 0) return ThrowRangeError(info, "Divide by zero");
		if (arg == -1 && value == MIN_VALUE) return ThrowRangeError(info, "Integer overflow");
		ReturnNew(info, value / arg);
	}
	
	NODE_METHOD(Modulo) { UseValue; UseArgument;
		if (arg == 0) return ThrowRangeError(info, "Divide by zero");
		ReturnNew(info, arg == -1 ? 0 : value % arg);
	}
	
	NODE_METHOD(Negate) { UseValue;
		if (value == MIN_VALUE) return ThrowRangeError(info, "Integer overflow");
		ReturnNew(info, -value);
	}
	
	NODE_METHOD(Abs) { UseValue;
		if (value == MIN_VALUE) return ThrowRangeError(info, "Integer overflow");
		ReturnNew(info, value >= 0 ? value : -value);
	}
	
	NODE_METHOD(And) { UseValue; UseArgument;
		ReturnNew(info, value & arg);
	}
	
	NODE_METHOD(Or) { UseValue; UseArgument;
		ReturnNew(info, value | arg);
	}
	
	NODE_METHOD(Xor) { UseValue; UseArgument;
		ReturnNew(info, value ^ arg);
	}
	
	NODE_METHOD(Not) { UseValue;
		ReturnNew(info, ~value);
	}
	
	NODE_METHOD(ShiftLeft) { UseValue;
		REQUIRE_ARGUMENT_UINT32(first, uint32_t shift);
		ReturnNew(info, (int64_t)((uint64_t)value << (shift & 63)));
	}
	
	NODE_METHOD(ShiftRight) { UseValue;
		REQUIRE_ARGUMENT_UINT32(first, uint32_t shift);
		ReturnNew(info, value >> (shift & 63));
	}
	
	NODE_METHOD(Equals) { UseValue; UseArgument;
		Return(info, value == arg);
	}
	
	NODE_METHOD(NotEquals) { UseValue; UseArgument;
		Return(info, value != arg);
	}
	
	NODE_METHOD(GreaterThan) { UseValue; UseArgument;
		Return(info, value > arg);
	}
	
	NODE_METHOD(GreaterThanOrEquals) { UseValue; UseArgument;
		Return(info, value >= arg);
	}
	
	NODE_METHOD(LessThan) { UseValue; UseArgument;
		Return(info, value < arg);
	}
	
	NODE_METHOD(LessThanOrEquals) { UseValue; UseArgument;
		Return(info, value <= arg);
	}
	
	NODE_METHOD(Compare) { UseValue; UseArgument;
		Return(info, value < arg ? -1 : value > arg ? 1 : 0);
	}
	
	NODE_METHOD(BitSizeAbs) { UseValue;
		if (value < 0) {
			if (value == MIN_VALUE) return Return(info, (uint32_t)64);
			value = -value;
		}
		uint64_t uvalue = (uint64_t)value;
		uint32_t bits = 1;
		while (uvalue >>= 1) {++bits;}
		Return(info, bits);
	}
	
	NODE_METHOD(IsEven) { UseValue;
		Return(info, (value & 1) == 0);
	}
	
	NODE_METHOD(IsOdd) { UseValue;
		Return(info, (value & 1) != 0);
	}
	
	NODE_METHOD(IsPositive) { UseValue;
		Return(info, value >= 0);
	}
	
	NODE_METHOD(IsNegative) { UseValue;
		Return(info, value < 0);
	}
	
	NODE_METHOD(IsZero) { UseValue;
		Return(info, value == 0);
	}
	
	NODE_METHOD(IsNonZero) { UseValue;
		Return(info, value != 0);
	}
	
	NODE_METHOD(IsSafe) { UseValue;
		Return(info, value <= MAX_SAFE && value >= MIN_SAFE);
	}
	
	NODE_METHOD(IsUnsafe) { UseValue;
		Return(info, value > MAX_SAFE || value < MIN_SAFE);
	}
	
	NODE_METHOD(ToNumberUnsafe) { UseValue;
		Return(info, (double)value);
	}
	
	NODE_METHOD(ToString) { UseValue;
		uint32_t radix = 10;
		if (info.Length()) {
			REQUIRE_ARGUMENT_UINT32(first, radix);
			if (radix < 2 || radix > 36) return ThrowRangeError(info, "Radix argument must be within 2 - 36");
		}
		char buffer[STRING_BUFFER_LENGTH];
		info.GetReturnValue().Set(StringFromLatin1(info.GetIsolate(), WriteString(buffer, value, (uint8_t)radix)));
	}
	
	NODE_METHOD(ValueOf) { UseValue;
		if (value <= MAX_SAFE && value >= MIN_SAFE) return Return(info, (double)value);
		char buffer[STRING_BUFFER_LENGTH];
		std::string message = "Cannot losslessly convert ";
		message += WriteString(buffer, value, 10);
		message += " to a number";
		ThrowRangeError(info, message.c_str());
	}
	
	static inline bool HasInstance(NODE_ARGUMENTS info, v8::Local<v8::Value> value) {
		return v8::Local<v8::FunctionTemplate>::New(info.GetIsolate(), constructorTemplate)->HasInstance(value);
	}
	
	static inline void Return(NODE_GETTER_ARGUMENTS info, int32_t value) {info.GetReturnValue().Set(value);}
	static inline void Return(NODE_ARGUMENTS info, int32_t value) {info.GetReturnValue().Set(value);}
	static inline void Return(NODE_ARGUMENTS info, uint32_t value) {info.GetReturnValue().Set(value);}
	static inline void Return(NODE_ARGUMENTS info, double value) {info.GetReturnValue().Set(value);}
	static inline void Return(NODE_ARGUMENTS info, bool value) {info.GetReturnValue().Set(value);}
	
	static inline void ReturnNew(NODE_ARGUMENTS info, int64_t value) {
		v8::Isolate* isolate = info.GetIsolate();
		constructing_privileges = true;
		constructing_value = value;
		info.GetReturnValue().Set(v8::Local<v8::Function>::New(isolate, constructor)->NewInstance(isolate->GetCurrentContext()).ToLocalChecked());
	}
	
	static Result Cast(NODE_ARGUMENTS info, v8::Local<v8::Value> value) {
		if (value->IsNumber()) return Cast(v8::Local<v8::Number>::Cast(value));
		if (value->IsString()) return Cast(v8::Local<v8::String>::Cast(value), 10);
		if (HasInstance(info, value)) return Result(node::ObjectWrap::Unwrap<Integer64>(v8::Local<v8::Object>::Cast(value))->value);
		return Result("Expected a number, string, or Integer64");
	}
	
	static Result Cast(v8::Local<v8::Number> number) {
		double value = number->Value();
		if (!isfinite(value) || floor(value) != value) return Result("The given number is not an integer");
		if (value > MAX_SAFE_DOUBLE || value < MIN_SAFE_DOUBLE) return Result("The precision of the given number cannot be guaranteed", true);
		return Result((int64_t)value);
	}
	
	static Result Cast(v8::Local<v8::String> string, uint8_t radix) {
		auto IsWhitespace = [](uint16_t c) {return c == ' ' || (c <= '\r' && c >= '\t');};
		
		v8::String::Value utf16(string);
		const uint16_t* str = *utf16;
		int len = utf16.length();
		int i = 0;
		
		// Skip leading whitespace.
		while (i<len && IsWhitespace(str[i])) {++i;}
		if (i == len) return Result("The given string does not contain a number");
		
		uint64_t value = 0;
		uint8_t max_digit = radix > 10 ? '9' : (radix - 1 + '0');
		uint8_t max_alpha = radix > 10 ? (radix - 11 + 'a') : 0;
		uint8_t min_alpha = radix > 10 ? 'a' : 255;
		bool is_negative = (i + 1 < len) && (str[i] == '-') && !IsWhitespace(str[i + 1]);
		int8_t sign = 1 - (is_negative * 2);
		for (i+=is_negative; i<len; ++i) {
			uint16_t c = str[i];
			if (c >= '0' && c <= max_digit) {
				uint64_t previous = value;
				if (previous > (value = value * radix + (c - '0'))) return Result("The given string represents a number that is too large", true);
			} else {
				if (c <= 'Z') c += 32;
				if (c >= min_alpha && c <= max_alpha) {
					uint64_t previous = value;
					if (previous > (value = value * radix + (c - 87))) return Result("The given string represents a number that is too large", true);
				} else {
					break;
				}
			}
		}
		
		// Skip trailing whitespace.
		while (i<len && IsWhitespace(str[i])) {++i;}
		
		if (i != len) return Result("The given string contains non-integer characters");
		if (value > I64_in_U64 + is_negative) return Result("The given string represents a number that is too large", true);
		return Result(((int64_t)value) * sign);
	}
	
	static char* WriteString(char* buffer, int64_t value, uint8_t radix) {
		int32_t sign = -(value < 0);
		uint64_t x = (value ^ sign) - sign;
		unsigned char* slot = reinterpret_cast<unsigned char*>(buffer + STRING_BUFFER_LENGTH - 1);
		*slot = '\0';
		
		do {
			uint64_t digit = x % radix;
			*(--slot) = digit + '0' + (digit > 9) * 39;
		} while (x /= radix);
		
		*(slot - 1) = '-';
		return reinterpret_cast<char*>(slot + sign);
	}
	
	static const int64_t MAX_VALUE = 0x7fffffffffffffffLL;
	static const int64_t MIN_VALUE = -0x8000000000000000LL;
	static const int64_t MAX_SAFE = 9007199254740991LL;
	static const int64_t MIN_SAFE = -9007199254740991LL;
	static constexpr double MAX_SAFE_DOUBLE = (double)MAX_SAFE;
	static constexpr double MIN_SAFE_DOUBLE = (double)MIN_SAFE;
	static constexpr uint64_t I64_in_U64 = (uint64_t)MAX_VALUE;
	static constexpr uint64_t U32_in_U64 = (uint64_t)0xffffffffLU;
	static const size_t STRING_BUFFER_LENGTH = 72;
	static v8::Persistent<v8::Function> constructor;
	static v8::Persistent<v8::FunctionTemplate> constructorTemplate;
	static bool constructing_privileges;
	static int64_t constructing_value;
	
	const int64_t value;
};

v8::Persistent<v8::Function> Integer64::constructor;
v8::Persistent<v8::FunctionTemplate> Integer64::constructorTemplate;
bool Integer64::constructing_privileges;
int64_t Integer64::constructing_value;

NODE_MODULE(integer64, Integer64::Init);
