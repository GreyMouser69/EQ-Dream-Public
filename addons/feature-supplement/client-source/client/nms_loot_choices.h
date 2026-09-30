#pragma once
#include <cctype>
#include <string>
namespace LootChoices {
struct Decision { unsigned char action; const char* rule; };
inline Decision Solo(int choice) {
 switch(choice) {
 case 1:return {1,nullptr}; case 2:return {2,nullptr};
 case 3:return {1,"Keep"}; case 4:return {2,"Sell"};
 default:return {0,nullptr};
 }
}
inline const char* Category(int choice) {
 static const char* names[]={"","Keep","Sell","Destroy","Bank","Vault","Tribute"};
 return choice>=0 && choice<7?names[choice]:"";
}
inline std::string Lower(std::string text) {
 for(auto& c:text)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
 return text;
}
inline std::string Trim(std::string text) {
 const auto b=text.find_first_not_of(" \t");
 if(b==std::string::npos) return "";
 const auto e=text.find_last_not_of(" \t");
 return text.substr(b,e-b+1);
}
inline bool ValidName(const std::string& text) {
 return !text.empty() && text.find_first_of("=\r\n[]")==std::string::npos;
}
inline std::string ResolveAddAction(const char* requested,const std::string& pending) {
 if(requested && *requested) return requested;
 return pending.empty()?"Keep":pending;
}
inline bool Matches(const std::string& key,const std::string& value,int category,const std::string& query) {
 const auto action=value.substr(0,value.find('|'));
 return (!*Category(category) || Lower(action)==Lower(Category(category))) &&
        (query.empty() || Lower(key).find(Lower(query))!=std::string::npos);
}
}
