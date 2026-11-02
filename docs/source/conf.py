from pathlib import Path
import sys


sys.path.insert(0, str(Path(__file__).resolve().parent))

project = "one_step_MAPF"
copyright = ""
author = "Jakub Oliver Kubín"
release = "main"

extensions = [
    "breathe",
    "sphinx.ext.graphviz",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "furo"
html_title = "one_step_MAPF documentation"
html_static_path = ["_static"]

breathe_projects = {
    "one_step_MAPF": "../build/doxygen/xml",
}
breathe_default_project = "one_step_MAPF"
breathe_default_members = ("members", "protected-members", "undoc-members")

todo_include_todos = True
