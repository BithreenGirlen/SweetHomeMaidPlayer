#ifndef SWMD_H_
#define SWMD_H_

#include <string>
#include <vector>

#include "adv.h"

namespace swmd
{
	bool LoadScenario(const std::wstring& wstrVoiceFolderPath, const std::wstring& wstrCardId, std::vector<adv::TextDatum>& textData);
}
#endif // !SWMD_H_
