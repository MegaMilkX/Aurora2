
#include "ext/simpleini/simpleini.h"


#include "util.h"

using json = nlohmann::json;



void create_main_cpp(std::string& source, const std::string& state_name)
{
    source.clear();
    
    source = 
    #include "core_main.cpp.template"
    ;
    
    replace_all(source, "%%STATE_NAME%%", state_name);
}

struct config
{
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
    std::vector<std::string> libraries;
};

struct project_config
{
    std::string build_path;
    std::string project_name;
    std::string source_path;
    std::string default_state;

    std::string root_dir;
};

bool load_config(config& conf)
{
    std::ifstream json_file(get_module_dir() + "\\config.json");
    json j;
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what();
        return 0;
    }
    if(!j.is_object())
    {
        std::cout << "config json is not object";
        return 0;
    }
    conf.include_paths = get_from_json_array(j["include_paths"]);
    conf.library_paths = get_from_json_array(j["library_paths"]);
    conf.libraries = get_from_json_array(j["libraries"]);

    return true;
}

bool load_project_json(const std::string& path, project_config& conf)
{
    std::ifstream json_file(path);
    json j;
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what();
        return false;
    }
    if(!j.is_object())
    {
        std::cout << "project config json is not object";
        return false;
    }
    
    conf.build_path = get_string(j["build_path"], "build");
    conf.project_name = get_string(j["project_name"], "UntitledProject");
    conf.source_path = get_string(j["source_path"], "");
    conf.default_state = get_string(j["default_state"], "");

    conf.root_dir = cut_dirpath(path);
    return true;
}

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

bool build(const project_config& proj_conf, const config& conf)
{
    CreateDirectoryA(
        MKSTR(proj_conf.root_dir << "\\" << proj_conf.build_path).c_str(),
        0
    );

    std::string include_paths_s;
    std::string library_paths_s;
    std::string libraries_s;
    for(auto v : conf.include_paths)
    {
        if(v.find_last_of(":") == v.npos)
        {
            v = get_module_dir() + "\\" + v;
        }
        include_paths_s += "/I \"" + v + "\" ";
    }
    for(auto v : conf.library_paths)
    {
        if(v.find_last_of(":") == v.npos)
        {
            v = get_module_dir() + "\\" + v;
        }        
        library_paths_s += "/LIBPATH:\"" + v + "\" ";
    }
    for(auto v : conf.libraries) 
        libraries_s += "\"" + v + "\" ";
    
    std::cout << include_paths_s << std::endl;
    std::cout << library_paths_s << std::endl;
    std::cout << libraries_s << std::endl;
    
    //std::cout << j.dump() << std::endl;
    
    std::string source_path = "\"" + get_module_dir() + "\\..\\..\\Source\\Core\\\" ";
    source_path += 
       "\"" + proj_conf.root_dir + "\\" + proj_conf.source_path + "\\\" ";
    source_path +=
        "\"" + get_module_dir() + "\\\"";
    std::cout << source_path;
    _putenv_s("INCLUDE_PATHS", include_paths_s.c_str());
    _putenv_s("LIB_PATHS", library_paths_s.c_str());
    _putenv_s("LIBRARIES", libraries_s.c_str());
    
    _putenv_s("EXENAME", proj_conf.project_name.c_str());
    _putenv_s("OUTDIR", (proj_conf.root_dir + "\\" "intermediate" "\\").c_str());
    _putenv_s("SOURCE_DIRS", source_path.c_str());
    
    if(system(("cd \"" + proj_conf.root_dir + "\" && call \"" + get_module_dir() + "\\build\"").c_str()) != 0)
    {
        std::cout << "Failed to build source" << std::endl;
        return false;
    }

    copy_file(
        MKSTR(proj_conf.root_dir << "\\intermediate\\" << proj_conf.project_name << ".exe"),
        MKSTR(proj_conf.root_dir << "\\" << proj_conf.build_path << "\\" << proj_conf.project_name << ".exe")
    );

    return true;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "no project path specified" << std::endl;
        return 0;
    }
    
    std::string project_filename = argv[1];

    config conf;
    load_config(conf);
    
    project_config proj_conf;
    load_project_json(project_filename, proj_conf);
    /*
    std::string main_src;
    create_main_cpp(main_src, proj_conf.default_state);
    std::ofstream file(get_module_dir() + "\\core_main.cpp");
    file << main_src;
    file.close();
    */
    std::cout << "Building " << proj_conf.project_name << std::endl;
    std::cout << get_module_dir() << std::endl;   
    
    if(!build(proj_conf, conf))
    {
        return 1;
    }

    if(system(("cd \"" + proj_conf.root_dir + "\" && call \"" + cut_dirpath(get_module_dir()) + "\\ResourceCompiler\\resource_compiler\" \"" + project_filename + "\"").c_str()) != 0)
    {
        std::cout << "Failed to build resources" << std::endl;
        return 1;
    }

    if(system(("cd \"" + proj_conf.root_dir + "\\" + proj_conf.build_path + "\" && call \"" + proj_conf.root_dir + "\\" + proj_conf.build_path + "\\" + proj_conf.project_name + "\"").c_str()) != 0)
    {
        std::cout << "Failed to launch built exe" << std::endl;
        return 1;
    }

    return 0;
}