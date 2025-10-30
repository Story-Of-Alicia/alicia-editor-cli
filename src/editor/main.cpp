#include <libpak/libpak.hpp>

#include <cstdio>
#include <iostream>
#include <ranges>
#include <filesystem>
#include <fstream>

namespace
{

void export_assets(libpak::resource& pak)
{
  for (const auto& asset : pak.assets | std::views::values)
  {
    if (asset.data.buffer.empty())
      continue;

    std::filesystem::path asset_path(asset.header.path);
    std::filesystem::create_directories(
      asset_path.parent_path());

    std::ofstream file(asset_path, std::ios::binary);
    if (!file.is_open())
    {
      printf("Couldn't write\n");
      return;
    }

    file.write(
      reinterpret_cast<const char*>(asset.data.buffer.data()),
      static_cast<int64_t>(asset.data.buffer.size()));
  }
}

void import_assets(libpak::resource& pak)
{
  for (auto& asset : pak.assets | std::views::values)
  {
    if (not asset.header.is_asset_embedded)
      continue;

    std::filesystem::path asset_path(asset.header.path);
    std::ifstream file(asset_path, std::ios::binary);
    if (!file.is_open())
    {
      continue;
    }

    wprintf(L"Patching '%ls'\n", reinterpret_cast<wchar_t*>(asset.header.path));

    file.seekg(0, std::ios::end);
    const uint32_t size = static_cast<uint32_t>(file.tellg());
    file.seekg(0);

    asset.data.buffer.resize(size);
    file.read(reinterpret_cast<char*>(asset.data.buffer.data()), size);

    asset.header.data_decompressed_length = size;
    asset.header.data_decompressed_length0 = size;
    asset.header.data_decompressed_length1 = size;
  }
}

void set_all_dev(libpak::resource& pak)
{
  for (auto& asset : pak.assets | std::views::values)
  {
    asset.header.is_asset_embedded = false;
    asset.header.embedded_data_length = 0;
    asset.header.embedded_data_offset = 0;
    asset.header.crc_embedded = 0x0;
    asset.header.checksum_embedded = 0x0;
  }
}

} // anon namespace

int main([[maybe_unused]] int argc, [[maybe_unused]] const char** args)
{
  printf("File: ");
  std::string file;
  std::getline(std::cin, file);

  libpak::resource pak(file);

  printf("Action [export, patch]: ");
  std::string action;
  std::getline(std::cin, action);

  if (action == "export")
  {
    printf("Reading..\n");
    pak.read(true);

    export_assets(pak);
    return 0;
  }
  else if (action == "patch")
  {
    printf("Reading..\n");
    pak.read(false);

    printf("Importing..\n");
    import_assets(pak);

    printf("Writing..\n");
    pak.resource_path = "res.pak.prod";
    pak.write();

    printf("Done.\n");
    return 0;
  }

  printf("No action '%s'\n", action.c_str());
  return 0;
}
