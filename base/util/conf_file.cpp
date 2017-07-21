#include <fstream>
#include "string_func.h"
#include "conf_file.h"

namespace Config 
{
	section::section()
	{
	}

	section::section(const string file_path, const string section)
	{
		m_file_path = file_path;
		m_section = section;
		inner_load();
	}

	section::section(const section& other)
	{
		m_file_path = other.m_file_path;
		m_section = other.m_section;
		m_value = other.m_value;
	}

	section::~section()
	{

	}

	void section::load(const string file_path, const string section)
	{
		m_file_path = file_path;
		m_section = section;
		m_value.clear();
		inner_load();
	}

	string section::get_name()
	{
		return m_section;
	}

	bool section::inner_load()
	{
		std::ifstream ifs;
		ifs.open(m_file_path.c_str(), std::ios::in);
		if (ifs.fail())
			return false;

		std::string app;
		std::string name;
		std::string value;
		std::string line;
		while (getline(ifs, line))
		{
			if (line.empty())
			{
				continue;
			}

			unsigned int i = 0;
			str_trim(line);

			char ch = line[i];
			if ('#' != ch && ';' != ch)
			{
				bool good = true;
				if ('[' == ch)
				{
					std::string::size_type  j = line.find(']', i);
					if (std::string::npos != j)
					{
						app = line.substr(i + 1, j - i - 1);
						str_trim(app);
						if (app.empty())
						{
							good = false;
						}
					}
					else
					{
						good = false;
					}
				}
				if (good && lowstring(app) == lowstring(m_section))
				{
					unsigned int j = (unsigned int)line.find('=', i);
					if (j > i)
					{
						name = line.substr(i, j - i);
						str_trim(name);
						if (!name.empty())
						{
							value = line.substr(j + 1);
							str_trim(value);
							m_value[name] = value;
						}
					}
				}
			}
		}
		return true;
	}

	string get_str(const string file_path, const string section_name, const string field_name)
	{
		section	sec(field_name, section_name);
		string str;
		sec.get(field_name, str);
		return str;
	}
}
