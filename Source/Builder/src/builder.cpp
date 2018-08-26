
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
    json bindings;

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

bool load_bindings(const std::string& path, json& j)
{
    std::ifstream json_file(path);
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        std::cout << (ex.what()) << std::endl;
        return false;
    }
    if(!j.is_object())
    {
        std::cout << ("Bindings json is not an object") << std::endl;
        return false;
    }

    return true;
}

bool load_project_json(const std::string& path, project_config& conf)
{
    conf.root_dir = cut_dirpath(path);

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

    json bindings = j["bindings"];
    if(bindings.is_string())
        load_bindings(conf.root_dir + "\\" + bindings.get<std::string>(), bindings);
    if(bindings.is_object())
        conf.bindings = bindings;
    
    conf.build_path = get_string(j["build_path"], "build");
    conf.project_name = get_string(j["project_name"], "UntitledProject");
    conf.source_path = get_string(j["source_path"], "");
    conf.default_state = get_string(j["default_state"], "");

    return true;
}

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

bool build(const project_config& proj_conf, const config& conf, bool build_editor = false)
{
    _putenv_s("ADDITIONAL_COMPILE_ARGS", "");
    std::string build_path;
    std::string target_name;

    if(build_editor)
    {
        _putenv_s("ADDITIONAL_COMPILE_ARGS", "/D AURORA2_BUILD_EDITOR");
        build_path = MKSTR(proj_conf.root_dir << "\\" << "editor");
        target_name = "scene_editor";
        CreateDirectoryA(build_path.c_str(), 0);
    }
    else
    {
        build_path = MKSTR(proj_conf.root_dir << "\\" << proj_conf.build_path);
        target_name = proj_conf.project_name;
        CreateDirectoryA(build_path.c_str(), 0);
    }

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
    
    _putenv_s("EXENAME", target_name.c_str());
    _putenv_s("OUTDIR", (proj_conf.root_dir + "\\" "intermediate" "\\").c_str());
    _putenv_s("SOURCE_DIRS", source_path.c_str());
    
    if(system(("cd \"" + proj_conf.root_dir + "\" && call \"" + get_module_dir() + "\\build\"").c_str()) != 0)
    {
        std::cout << "Failed to build source" << std::endl;
        return false;
    }

    copy_file(
        MKSTR(proj_conf.root_dir << "\\intermediate\\" << target_name << ".exe"),
        MKSTR(build_path << "\\" << target_name << ".exe")
    );

    return true;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "Not enough parameters. At least specify your project config" << std::endl;
        return 0;
    }

    std::string project_filename;
    bool build_editor = false;
    for(int i = 1; i < argc; ++i)
    {
        std::string param(argv[i]);
        if(param.compare(0, 1, "-") == 0)
        {
            if(param == "-editor") build_editor = true;
        }
        else
        {
            project_filename = argv[i];
        }
    }
    
    if(project_filename.empty())
    {
        std::cout << "No project config specified. Abort." << std::endl;
        return 0;
    }

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
    if(build_editor)
    {
        std::cout << "Building " << proj_conf.project_name << " editor" << std::endl;
    }
    else
    {
        std::cout << "Building " << proj_conf.project_name << std::endl;
    }
    std::cout << get_module_dir() << std::endl;   

    if(!build(proj_conf, conf, build_editor))
    {
        return 1;
    }

    if(!build_editor)
    {
        if(system(("cd \"" + proj_conf.root_dir + "\" && call \"" + cut_dirpath(get_module_dir()) + "\\ResourceCompiler\\resource_compiler\" \"" + project_filename + "\"").c_str()) != 0)
        {
            std::cout << "Failed to build resources" << std::endl;
            return 1;
        }
    }

    if(proj_conf.bindings.is_object())
    {
        std::ofstream f(proj_conf.root_dir + "\\" + proj_conf.build_path + "\\bindings.json");
        std::cout << (proj_conf.root_dir + "\\" + proj_conf.build_path + "\\bindings.json") << std::endl;
        if(f.is_open())
        {
            f << proj_conf.bindings.dump(4);
        }
        else
        {
            std::cout << "Failed to open bindings.json" << std::endl;
        }
        f.close();
    }

    // Launch game
    if(!build_editor)
    {
        if(system(("cd \"" + proj_conf.root_dir + "\\" + proj_conf.build_path + "\" && call \"" + proj_conf.root_dir + "\\" + proj_conf.build_path + "\\" + proj_conf.project_name + "\"").c_str()) != 0)
        {
            std::cout << "Failed to launch built exe" << std::endl;
            return 1;
        }
    }

    return 0;
}