import os
import shutil
import xml.etree.ElementTree as ET
import uuid

# Define paths
project_dir = r"c:\Users\yuuhi\Cpp_Project\Study\DirectXGame_FeatureLab\DirectX_Render_Study\DirectX12_BoxDraw_Refactoring"
external_dir = os.path.join(project_dir, "Source", "External")
vcxproj_path = os.path.join(project_dir, "DirectX12_BoxDraw_Refactoring.vcxproj")
filters_path = os.path.join(project_dir, "DirectX12_BoxDraw_Refactoring.vcxproj.filters")

# Files to move to Source/External
files_to_move = [
    "imgui.cpp", "imgui.h", "imgui_demo.cpp", "imgui_draw.cpp", "imgui_impl_dx12.cpp",
    "imgui_impl_dx12.h", "imgui_impl_win32.cpp", "imgui_impl_win32.h", "imgui_internal.h",
    "imgui_tables.cpp", "imgui_widgets.cpp", "imconfig.h",
    "imstb_rectpack.h", "imstb_textedit.h", "imstb_truetype.h",
    "json.hpp", "stb_image.h", "stb_image_write.h", "tiny_gltf.h", "tinygltf_json.h"
]

# 1. Move files
print("Moving files...")
for file in files_to_move:
    src = os.path.join(project_dir, file)
    dst = os.path.join(external_dir, file)
    if os.path.exists(src):
        shutil.move(src, dst)
        print(f"Moved {file} to Source\\External")
    else:
        print(f"Skipped {file} (not found)")

# 2. Update .vcxproj
print("\nUpdating .vcxproj...")
ET.register_namespace('', 'http://schemas.microsoft.com/developer/msbuild/2003')
tree = ET.parse(vcxproj_path)
root = tree.getroot()
ns = {'msbuild': 'http://schemas.microsoft.com/developer/msbuild/2003'}

# Track all items that need filters
project_items = [] # list of (item_type, file_path)

for item_group in root.findall('msbuild:ItemGroup', ns):
    for item_type in ['ClCompile', 'ClInclude', 'FxCompile', 'ResourceCompile']:
        for item in item_group.findall(f'msbuild:{item_type}', ns):
            inc = item.get('Include')
            if inc:
                filename = os.path.basename(inc)
                if filename in files_to_move:
                    new_path = f"Source\\External\\{filename}"
                    item.set('Include', new_path)
                    print(f"Updated vcxproj entry: {inc} -> {new_path}")
                    project_items.append((item_type, new_path))
                else:
                    project_items.append((item_type, inc))

tree.write(vcxproj_path, encoding="utf-8", xml_declaration=True)

# 3. Generate .vcxproj.filters
print("\nGenerating .vcxproj.filters...")
filter_tree = ET.Element('Project', {'ToolsVersion': '4.0', 'xmlns': 'http://schemas.microsoft.com/developer/msbuild/2003'})

# Extract all required directories (filters)
directories = set()
for _, path in project_items:
    dir_path = os.path.dirname(path)
    # Split by \ and add all parent directories
    parts = dir_path.split('\\')
    for i in range(1, len(parts) + 1):
        sub_dir = '\\'.join(parts[:i])
        if sub_dir:
            directories.add(sub_dir)

# Create filter definitions
filters_group = ET.SubElement(filter_tree, 'ItemGroup')
for d in sorted(list(directories)):
    filter_elem = ET.SubElement(filters_group, 'Filter', {'Include': d})
    guid_elem = ET.SubElement(filter_elem, 'UniqueIdentifier')
    guid_elem.text = '{' + str(uuid.uuid4()) + '}'

# Create item groups for files
items_groups = {
    'ClCompile': ET.SubElement(filter_tree, 'ItemGroup'),
    'ClInclude': ET.SubElement(filter_tree, 'ItemGroup'),
    'FxCompile': ET.SubElement(filter_tree, 'ItemGroup'),
    'ResourceCompile': ET.SubElement(filter_tree, 'ItemGroup')
}

for item_type, path in project_items:
    group = items_groups[item_type]
    elem = ET.SubElement(group, item_type, {'Include': path})
    
    dir_path = os.path.dirname(path)
    if dir_path:
        filter_node = ET.SubElement(elem, 'Filter')
        filter_node.text = dir_path

# Remove empty ItemGroups
for item_type, group in items_groups.items():
    if len(group) == 0:
        filter_tree.remove(group)

# Pretty formatting function
def indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

indent(filter_tree)
with open(filters_path, "wb") as f:
    f.write(b'<?xml version="1.0" encoding="utf-8"?>\n')
    ET.ElementTree(filter_tree).write(f, encoding="utf-8", xml_declaration=False)

print("Done.")
