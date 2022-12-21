#pragma once

template<>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<uint16_t>(const uint16_t& v)
{
    RETURN_WIDE_STRING(v);
}

template<>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<int64_t>(const int64_t& v)
{
    RETURN_WIDE_STRING(v);
}

//template<>
//inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<uint64_t>(const uint64_t& v)
//{
//    RETURN_WIDE_STRING(v);
//}
