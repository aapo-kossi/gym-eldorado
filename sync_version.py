import re
import toml
from pathlib import Path

project_dir = Path(__file__).parent

# Load version from pyproject.toml
with open(project_dir / "pyproject.toml", "r") as f:
    pyproject = toml.load(f)
    version = pyproject["project"]["version"]

# Update CMakeLists.txt
cmake_file = "CMakeLists.txt"
with open(project_dir / cmake_file, "r") as f:
    cmake_content = f.read()

# Replace the version in CMakeLists.txt (assuming a `set(PROJECT_VERSION "X.Y.Z")` pattern)
cmake_content_new = re.sub(r'VERSION\s+"[\d.]+"\s', f'VERSION "{version}" ', cmake_content)

if cmake_content_new != cmake_content:
    with open(project_dir / cmake_file, "w") as f:
        f.write(cmake_content_new)
    print(f"Updated CMakeLists.txt version to {version}")
else:
    print("CMakeLists.txt is already up-to-date")

