import re
import toml

# Load version from pyproject.toml
with open("pyproject.toml", "r") as f:
    pyproject = toml.load(f)
    version = pyproject["project"]["version"]

# Update CMakeLists.txt
cmake_file = "CMakeLists.txt"
with open(cmake_file, "r") as f:
    cmake_content = f.read()

# Replace the version in CMakeLists.txt (assuming a `set(PROJECT_VERSION "X.Y.Z")` pattern)
cmake_content_new = re.sub(r'VERSION\s+"[\d.]+"\s', f'VERSION "{version}" ', cmake_content)

if cmake_content_new != cmake_content:
    with open(cmake_file, "w") as f:
        f.write(cmake_content_new)
    print(f"Updated CMakeLists.txt version to {version}")
else:
    print("CMakeLists.txt is already up-to-date")

