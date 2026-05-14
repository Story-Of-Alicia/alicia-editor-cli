#include <libpak/libpak.hpp>

#include <cstdio>
#include <iostream>
#include <ranges>
#include <filesystem>
#include <fstream>

namespace
{

void export_assets(libpak::resource& pak, bool ignoreExisting = true)
{
  for (const auto& asset : pak.assets | std::views::values)
  {
    if (asset.data.buffer.empty())
      continue;

    std::filesystem::path asset_path(asset.header.path);
    if (std::filesystem::exists(asset_path) && ignoreExisting)
      continue;

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
    std::filesystem::path asset_path(asset.header.path);

    const bool isExecutable = asset_path.extension() == ".exe";
    const bool isLibrary = asset_path.extension() == ".dll";
    const bool isMovie = asset_path.extension() == ".avi";
    const bool isAuxiliary = asset_path.extension() == ".txt"
      || asset_path.extension() == ".ini";

    if (not isExecutable && not isLibrary && not isMovie && not isAuxiliary)
    {
      asset.header.are_data_embedded = 1;
    }

    std::ifstream file(asset_path, std::ios::binary);
    if (!file.is_open())
    {
      wprintf(L"Asset '%ls' is missing it's data file\n", reinterpret_cast<wchar_t*>(asset.header.path));
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
    asset.header.are_data_embedded = false;
    asset.header.embedded_data_length = 0;
    asset.header.embedded_data_offset = 0;
    asset.header.crc_embedded = 0x0;
    asset.header.checksum_embedded = 0x0;
  }
}

const std::vector<std::u16string> filter = {
  u"emoticon_agent",
};

void only_take_filer(libpak::resource& pak)
{
  for (auto assetIter = pak.assets.begin(); assetIter != pak.assets.end();)
  {
    auto& asset = assetIter->second;

    bool keep = false;
    for (const auto& token : filter)
    {
      keep = asset.path().find(token) != std::u16string::npos;
      if (keep)
        break;
    }

    if (keep)
    {
      ++assetIter;
    }
    else
    {
      assetIter = pak.assets.erase(assetIter);
    }
  }
}

void debug_asset(const libpak::asset& asset)
{
    wprintf(L"Asset: %ls\n", reinterpret_cast<const wchar_t*>(asset.header.path));
    wprintf(L"\t prefix: 0x%X\n", asset.header.prefix);
    wprintf(L"\t path length: 0x%X\n", asset.header.path_length);
    wprintf(L"\t embedded data offset: 0x%X\n", asset.header.embedded_data_offset);
    wprintf(L"\t embedded data length: 0x%X\n", asset.header.embedded_data_length);
    wprintf(L"\t data decompressed length: 0x%X\n", asset.header.data_decompressed_length);
    wprintf(L"\t are data compressed: %s\n", asset.header.are_data_compressed ? L"yes" : L"no");
    wprintf(L"\t data decompressed length 0: 0x%X\n", asset.header.data_decompressed_length0);
    wprintf(L"\t unknown 0: 0x%X\n", asset.header.unknown0);
    wprintf(L"\t data decompressed length 1: 0x%X\n", asset.header.data_decompressed_length1);
    wprintf(L"\t timestamp: 0x%X\n", asset.header.timestamp);
    wprintf(L"\t path hash: 0x%X\n", asset.header.path_hash);
    wprintf(L"\t filename hash: 0x%X\n", asset.header.filename_hash);
    wprintf(L"\t extension hash: 0x%X\n", asset.header.extension_hash);
    wprintf(L"\t parent path hash: 0x%X\n", asset.header.parent_path_hash);
    wprintf(L"\t are data deleted: %s\n", asset.header.is_asset_deleted ? L"yes" : L"no");
    wprintf(L"\t asset header offset: 0x%X\n", asset.header.header_offset);
    wprintf(L"\t are asset data embedded: %s\n", asset.header.are_data_embedded  ? L"yes" : L"no");
    wprintf(L"\t unknown_type_l: 0x%X\n", asset.header.unknown_type_l);
    wprintf(L"\t unknown_type_h: 0x%X\n", asset.header.unknown_type_h);
    wprintf(L"\t unknown_value_l: 0x%X\n", asset.header.date_created);
    wprintf(L"\t unknown_value_h: 0x%X\n", asset.header.time_created);
    wprintf(L"\t crc decompressed: 0x%X\n", asset.header.crc_decompressed);
    wprintf(L"\t crc embedded: 0x%X\n", asset.header.crc_embedded);
    wprintf(L"\t crc identity: 0x%X\n", asset.header.crc_identity);
    wprintf(L"\t checksum decompressed: 0x%X\n", asset.header.checksum_decompressed);
    wprintf(L"\t checksum embedded: 0x%X\n", asset.header.checksum_embedded);
    wprintf(L"\t unknown 6: 0x%X\n", asset.header.unknown6);
}

} // anon namespace

int main([[maybe_unused]] int argc, [[maybe_unused]] const char** args)
{
  printf("File: ");
  std::string file;
  std::getline(std::cin, file);

  libpak::resource pak(file);

  printf("Action [info, export, unpack, repack, strip, merge]: ");
  std::string action;
  std::getline(std::cin, action);

  if (action == "info")
  {
    printf("Reading...\n");
    pak.read(false);

    for (const auto& asset : pak.assets | std::views::values)
    {
      debug_asset(asset);
    }

    printf("You exported the assets to the current working directory.\n");
    return 0;
  }
  if (action == "export")
  {
    printf("Reading...\n");
    pak.read(true);

    printf("Exporting...\n");
    export_assets(pak, false);

    printf("You exported the assets to the current working directory.\n");
    return 0;
  }
  else if (action == "unpack")
  {
    printf("Reading...\n");
    pak.read(true);

    printf("Exporting...\n");
    export_assets(pak);

    printf("Patching headers...\n");
    set_all_dev(pak);

    printf("Writing...");
    pak.resource_path += ".dev";
    pak.write();

    printf("Done.\n");
    printf("You now have '%s' which is unpacked.\n", pak.resource_path.c_str());
    printf("To actually use it, rename it to 'res.pak'.\n");
    return 0;
  }
  else if (action == "repack")
  {
    printf("Reading...\n");
    pak.read(false);

    printf("Importing...\n");
    import_assets(pak);

    printf("Writing...\n");
    pak.resource_path += ".packed";
    pak.write();

    printf("Done.\n");
    printf("You now have '%s' which is packed.\n", pak.resource_path.c_str());
    printf("To actually use it, rename it to 'res.pak'.\n");
    return 0;
  }
  else if (action == "strip")
  {
    printf("Reading...\n");
    pak.read(true);

    printf("Patching headers...\n");
    only_take_filer(pak);

    printf("Writing...");
    pak.resource_path = "res.pak.stripped";
    pak.write();

    printf("Done.\n");
    printf("You now have '%s' which is stripped of certain assets.\n", pak.resource_path.c_str());
    printf("To actually use it, rename it to 'res.pak'.\n");
    return 0;
  }
  else if (action == "merge")
  {
    printf("Reading...\n");
    pak.read(true);

    printf("Other file to merge: ");
    std::getline(std::cin, file);

    libpak::resource otherPak(file);
    printf("Reading other pak...\n");
    otherPak.read(true);

    for (auto& asset : otherPak.assets)
    {
      wprintf(L"Merging '%ls'\n", reinterpret_cast<wchar_t*>(asset.second.header.path));
      pak.assets[asset.second.path()] = std::move(asset.second);
    }

    printf("Writing...");
    pak.resource_path = "res.pak.merged";
    pak.write();

    printf("Done.\n");
    printf("You now have '%s' which is a result of a merge.\n", pak.resource_path.c_str());
    printf("To actually use it, rename it to 'res.pak'.\n");
    return 0;
  }

  printf("No action '%s'\n", action.c_str());
  return 0;
}
