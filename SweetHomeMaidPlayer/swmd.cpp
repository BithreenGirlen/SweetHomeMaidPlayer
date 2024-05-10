

#include "swmd.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

namespace swmd
{
	void TextToLines(const std::wstring& wstrText, const wchar_t* wpzKey, size_t nKeyLen, std::vector<std::wstring>& lines)
	{
		if (wpzKey == nullptr)return;

		size_t nRead = 0;
		for (;;)
		{
			const wchar_t* p = wcsstr(&wstrText[nRead], wpzKey);
			if (p == nullptr)break;

			size_t nLen = p - &wstrText[nRead];
			lines.emplace_back(wstrText.substr(nRead, nLen));
			nRead += nLen + nKeyLen;
			if (nRead >= wstrText.size())break;
		}
	}

	void TruncateLines(std::vector<std::wstring>& lines)
	{
		for (auto& line : lines)
		{
			size_t nPos = line.find(L"\r\n");
			if (nPos == std::wstring::npos)continue;
			size_t nLen = line.size() - nPos;

			const std::vector<std::wstring> refs = { L"\r\n\r\n" , L"@", L"//" };
			wchar_t* p = nullptr;
			for (const auto& ref : refs)
			{
				wchar_t* q = wcsstr(&line[nPos], ref.c_str());
				if (q != nullptr)
				{
					if (p == nullptr)p = q;
					else p = p < q ? p : q;
				}
			}
			if (p != nullptr)
			{
				nLen = p - &line[nPos];
			}
			line = line.substr(nPos + 2, nLen - 2); // Adjust by the length of "\r\n"
		}
	}

	std::wstring GetEpisodeJsonPath(const std::wstring& wstrVoiceFolderPath, const std::wstring& wstrCardId)
	{
		size_t nPos = wstrVoiceFolderPath.rfind(L"Resource");
		if (nPos == std::wstring::npos)return std::wstring();
		std::wstring wstrEpisodeJson = wstrVoiceFolderPath.substr(0, nPos);
		wstrEpisodeJson += L"Episode\\Card\\story";
		wstrEpisodeJson += wstrCardId;
		wstrEpisodeJson += L"3.json";
		return wstrEpisodeJson;
	}

	std::wstring TruncateFilePath(std::wstring& wstrFilePath)
	{
		size_t nPos = wstrFilePath.rfind(L'/');
		if (nPos == std::wstring::npos)return wstrFilePath;

		return wstrFilePath.substr(nPos + 1);
	}
}

bool swmd::LoadScenario(const std::wstring& wstrVoiceFolderPath, const std::wstring& wstrCardId, std::vector<adv::TextDatum>& textData)
{
	std::wstring wstrEpisodeJson = GetEpisodeJsonPath(wstrVoiceFolderPath, wstrCardId);

	std::string strFile = win_filesystem::LoadFileAsString(wstrEpisodeJson.c_str());
	if (strFile.empty())return false;

	std::string strError;
	std::wstring wstrScenario;

	try
	{
		nlohmann::json nlJson = nlohmann::json::parse(strFile);
		wstrScenario = win_text::WidenUtf8(nlJson.at(5).at(0).at(2));
	}
	catch (nlohmann::json::exception e)
	{
		strError = e.what();
	}

	if (!strError.empty())
	{
		win_dialogue::ShowMessageBox("Parse error", strError.c_str());
	}

	std::vector<std::wstring> lines;
	constexpr wchar_t wszSeparator[] = L"@ShowCastMessage,";
	TextToLines(wstrScenario, wszSeparator, sizeof(wszSeparator) / sizeof(wchar_t) - 1, lines);
	constexpr wchar_t wszSeparator2[] = L"@ShowMessage,";
	TextToLines(wstrScenario, wszSeparator2, sizeof(wszSeparator2) / sizeof(wchar_t) - 1, lines);
	TruncateLines(lines);

	for (const auto& line : lines)
	{
		if (line.empty())continue;

		std::wstring wstrVoiceFilePath;
		std::wstring wstrMessage;

		size_t nRead = 0;

		if (line.size() > 2 && line.at(0) == '$' && line.at(1) == '$')
		{
			/*âπê∫éwíË*/
			nRead += 2;
			const wchar_t* p = wcsstr(&line[nRead], L"\r\n");
			if (p != nullptr)
			{
				size_t nLen = p - &line[nRead];
				std::wstring wstr = line.substr(nRead, nLen);
				wstrVoiceFilePath = wstrVoiceFolderPath + L"\\" + TruncateFilePath(wstr) + L".mp3";
				nRead += nLen + 2;
			}
		}

		size_t nLen = 0;
		const wchar_t* p = wcsstr(&line[nRead], L"\r\n\r\n");
		if (p != nullptr)
		{
			nLen = p - &line[nRead];
		}
		else
		{
			nLen = line.size() - nRead;
		}

		wstrMessage = line.substr(nRead, nLen);

		textData.emplace_back(adv::TextDatum{ wstrMessage, wstrVoiceFilePath });
	}

	return !textData.empty();
}
