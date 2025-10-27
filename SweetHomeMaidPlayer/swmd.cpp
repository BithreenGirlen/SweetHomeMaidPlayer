

#include "swmd.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "json_minimal.h"
#include "text_utility.h"

namespace swmd
{
	static std::wstring ExtractEpisodeScript(const std::wstring& wstrFilePath)
	{
		std::wstring wstrScenario;
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (!strFile.empty())
		{
			char* p1 = &strFile[0];
			const size_t indices[] = { 5, 0, 2 };
			char* p2 = nullptr;
			bool bRet = json_minimal::ExtractArrayValueByIndices(p1, indices, sizeof(indices) / sizeof(indices[0]), &p2);
			if (bRet)
			{
				wstrScenario = win_text::WidenUtf8(p2);
				free(p2);

				text_utility::ReplaceAll(wstrScenario, L"\\r", L"\r");
				text_utility::ReplaceAll(wstrScenario, L"\\n", L"\n");
				text_utility::ReplaceAll(wstrScenario, L"\\t", L"");
				text_utility::ReplaceAll(wstrScenario, L"\"", L"");
			}
		}

		return wstrScenario;
	}

	static void TextToLines(const std::wstring& wstrText, const wchar_t* wpzKey, size_t nKeyLen, std::vector<std::wstring>& lines)
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

	static void TruncateLines(std::vector<std::wstring>& lines)
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

	static std::wstring GetEpisodeJsonPath(const std::wstring& wstrVoiceFolderPath, const std::wstring& wstrCardId)
	{
		size_t nPos = wstrVoiceFolderPath.rfind(L"Resource");
		if (nPos == std::wstring::npos)return std::wstring();
		std::wstring wstrEpisodeJson = wstrVoiceFolderPath.substr(0, nPos);
		wstrEpisodeJson += L"Episode\\Card\\story";
		wstrEpisodeJson += wstrCardId;
		wstrEpisodeJson += L"3.json";
		return wstrEpisodeJson;
	}

	static std::wstring TruncateFilePath(const std::wstring& wstrFilePath)
	{
		size_t nPos = wstrFilePath.rfind(L'/');
		if (nPos == std::wstring::npos)return wstrFilePath;

		return wstrFilePath.substr(nPos + 1);
	}

	static std::wstring TruncateCardAudioFilePath(const std::wstring& wstrFilePath)
	{
		static constexpr const wchar_t key[] = L"Card/card";
		static constexpr const size_t keyLen = sizeof(key) / sizeof(wchar_t) - 1;
		size_t nPos = wstrFilePath.find(key);
		if (nPos == std::wstring::npos)return wstrFilePath;

		return wstrFilePath.substr(nPos + keyLen);
	}
}

bool swmd::LoadScenario(const std::wstring& wstrVoiceFolderPath, const std::wstring& wstrCardId, std::vector<adv::TextDatum>& textData)
{
	std::wstring wstrEpisodeJsonPath = GetEpisodeJsonPath(wstrVoiceFolderPath, wstrCardId);
	std::wstring wstrScenario = ExtractEpisodeScript(wstrEpisodeJsonPath);

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
			/*音声指定*/
			nRead += 2;
			const wchar_t* p = wcsstr(&line[nRead], L"\r\n");
			if (p != nullptr)
			{
				size_t nLen = p - &line[nRead];
				std::wstring wstr = line.substr(nRead, nLen);
				wstrVoiceFilePath = wstrVoiceFolderPath + TruncateCardAudioFilePath(wstr) + L".mp3";
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
