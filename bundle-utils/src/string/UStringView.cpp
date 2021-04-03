//
// Created by Duzhong Chen on 2021/4/4.
//

#include "UStringView.h"
#include "PrivateStringUtils.h"

bool operator==(UStringView lhs, UStringView rhs) noexcept {
    return lhs.size() == rhs.size() &&
        PrivateStringUtils::ucstrcmp(lhs.m_data, lhs.m_size, rhs.m_data, rhs.m_size) == 0;
}

UStringView UStringView::mid(uint32_t pos, uint32_t n) const noexcept
{
    auto result = PrivateStringUtils::ContainerImplHelper::mid(size(), &pos, &n);
    return result == PrivateStringUtils::ContainerImplHelper::Null ? UStringView() : UStringView(m_data + pos, n);
}
