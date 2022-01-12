#include <string>
#include <vector>
#include <dirent.h>

void readbmps(const std::string &dirname, std::vector<std::string> &output)
{
  DIR *dir = opendir(dirname.c_str());
  struct dirent *ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    if (ent->d_type == DT_REG)
    {
      std::string name = ent->d_name;
      if (name.substr(name.find_last_of(".") + 1) == "bmp")
        output.push_back(name);
    }
  }
  closedir(dir);
}
