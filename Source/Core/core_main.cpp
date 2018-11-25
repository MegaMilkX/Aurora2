#include <game_state.h>
#include <component.h>

#include <transform.h>
#include <camera.h>
#include <model.h>
#include <light_omni.h>
#include <sound_emitter.h>
#include <text_mesh.h>

#include "editor/editor_config.h"

#define MINIZ_HEADER_FILE_ONLY
#include "../lib/miniz.c"
void InitArchiveResources()
{
    std::vector<std::string> files = 
        find_all_files(get_module_dir() + "\\data", "*.bin");
    for(auto f : files){
        LOG(f);
    }

    for(auto f : files){
        std::shared_ptr<mz_zip_archive> zip(new mz_zip_archive);
        memset(zip.get(), 0, sizeof(mz_zip_archive));
        if(!mz_zip_reader_init_file(zip.get(), f.c_str(), 0)) {
            LOG_ERR("Failed to open " << f);
            continue;
        }

        mz_uint n_files = mz_zip_reader_get_num_files(zip.get());
        LOG(f << " has " << n_files << " entries");

        for(unsigned i = 0; i < n_files; ++i) {
            mz_zip_archive_file_stat f_stat;
            if(!mz_zip_reader_file_stat(zip.get(), i, &f_stat)) {
                LOG_ERR("Failed to get file stat for file at index " << i << " in " << f);
                continue;
            }

            GlobalDataRegistry().Add(
                f_stat.m_filename,
                DataSourceRef(new DataSourceArchive(f_stat.m_file_index, zip))
            );
        }
    }
}

void InitFilesystemResources(const std::string& rootDir) {
    std::vector<std::string> files =
        find_all_files(rootDir, "*.scn;*.geo;*.anim;*.mat;*.png;*.jpg;*.jpeg");
    std::vector<std::string> resNames = files;
    for(auto& f : resNames) {
        f.erase(f.find_first_of(rootDir), rootDir.size());
        if(f[0] == '\\') f.erase(0, 1);
        std::replace(f.begin(), f.end(), '\\', '/');
    }

    for(size_t i = 0; i < files.size(); ++i) {
        GlobalDataRegistry().Add(
            resNames[i],
            DataSourceRef(new DataSourceFilesystem(files[i]))
        );
    }
}

void Aurora2Init();

int editor_main(int argc, char** argv)
{
    EditorConfig().Init();
    InitFilesystemResources(EditorConfig().projectRoot + "\\resources");
    GameState::InitEditor();

    rttr::array_range<rttr::type> list = rttr::type::get<Resource>().get_derived_classes();
    LOG("Resource types: ");
    for(auto t : list) {
        LOG(t.get_name().to_string());
    }

    while(GameState::UpdateEditor())
    {
        glfwPostEmptyEvent();
        GameState::UpdateEditor();
        glfwPostEmptyEvent();
        GameState::UpdateEditor();
    }
    GameState::Cleanup();
    return 0;
}

int game_main(int argc, char** argv)
{
    InitArchiveResources();
    
    GameState::Init();
    Aurora2Init();
    while(GameState::Update())
    {}
    GameState::Cleanup();
    return 0;
}

int main(int argc, char** argv)
{
#ifdef AURORA2_BUILD_EDITOR
    return editor_main(argc, argv);
#else
    return game_main(argc, argv);
#endif
}