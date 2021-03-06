//	DX11Renderer - VDemo | DirectX11 Renderer
//	Copyright(C) 2016  - Volkan Ilbeyli
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.
//
//	Contact: volkanilbeyli@gmail.com

#pragma once

#include <string>
#include <fstream>

enum EErrorLog
{
	CANT_OPEN_FILE,
	CANT_CREATE_RESOURCE,
	CANT_CRERATE_RENDER_STATE,

	ERR_LOG_COUNT
};

constexpr size_t LEN_MSG_BUFFER = 2048;

class Log
{
public:
	Log()  = delete;
	~Log() = delete;

	static void Initialize(bool bEnableLogging);
	static void Exit();

	static void Info(const std::string& s);
	static void Error(const std::string& s);
	static void String(const std::string& s);
	static void Warning(const std::string& s);

	static void Error(EErrorLog errMode, const std::string& s);

	// functions for printf() style logging using variadic templates
	template<class... Args> 
	static void Error(const char* format, Args&&... args)
	{
		char msg[LEN_MSG_BUFFER];	sprintf_s(msg, format, args...);
		Error(std::string(msg));
	}

	template<class... Args>
	static void Warning(const char* format, Args&&... args)
	{
		char msg[LEN_MSG_BUFFER];	sprintf_s(msg, format, args...);
		Warning(std::string(msg));
	}

	// adds a new line 
	template<class... Args>
	static void Info(const char* format, Args&&... args)
	{
		char msg[LEN_MSG_BUFFER];	sprintf_s(msg, format, args...);
		Info(std::string(msg));
	}

	// prints as is 
	template<class... Args>
	static void String(const char* format, Args&&... args)
	{
		char msg[LEN_MSG_BUFFER];	sprintf_s(msg, format, args...);
		String(std::string(msg));
	}
private:
	static std::ofstream sOutFile;	
};

