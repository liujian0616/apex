#ifndef CONF_FILE_H_
#define CONF_FILE_H_

#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <map>
#include <sstream>

using namespace std;

namespace Config
{
	class section
	{
	public:
		section();
		section(const string file_path, const string section);
		section(const section& other);
		virtual ~section();

		void		load(const string file_path, const string section);
		string		get_name();

		template<class T>
		T& get(const std::string& name, T& value, const std::string& def = "")
		{
			std::string val;

			KEY_VALUE::iterator subit = m_value.find(name);
			if (subit != m_value.end())
			{
				val = subit->second;
			}
			else
			{
				val = def;
			}
			std::stringstream sstream(val);
			sstream >> value;

			return value;
		}
	private:
		bool inner_load();

		string		m_file_path;
		string		m_section;

		typedef map< std::string, std::string > KEY_VALUE;
		KEY_VALUE m_value;
	};

	string get_str(const string file_path, const string section_name, const string field_name);
}

#endif //CONF_FILE_H_