# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import json
from datetime import datetime, timezone

# -- Build info (from CMake) -------------------------------------------------
with open('build_info.json') as f:
    build_info = json.load(f)
    build_info['timestamp'] = datetime.fromtimestamp(build_info['timestamp'], tz=timezone.utc)

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = build_info['name']
copyright = f'{build_info['timestamp'].year}, {build_info['author']}'
author = build_info['author']
version = build_info['version']

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = []

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'CMakeLists.txt', 'requirements.txt', '*.json']

rst_prolog = f"""
.. |display_name| replace:: {build_info['display_name']}
.. |version| replace:: {version}
"""

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# https://github.com/executablebooks/sphinx-book-theme
html_theme = 'sphinx_book_theme'
html_static_path = ['_static']
html_title = '{name} v{version}'.format(name=build_info['display_name'], version=version)
html_copy_source = False
html_domain_indices = False
html_show_sourcelink = False
html_show_sphinx = False
html_show_copyright = False
html_theme_options = {
    'extra_footer': f'<p>Generated from commit <a href="https://github.com/danielskeenan/sacnlogger/commit/{build_info['commit']}">{build_info['commit']}</a></p>',
    'logo': {
        'text': html_title,
    },
    'navigation_with_keys': False,
    'path_to_docs': 'doc/',
    'repository_branch': 'main',
    'repository_url': 'https://github.com/danielskeenan/sacnlogger',
    'use_download_button': False,
    'use_edit_page_button': False,
    'use_fullscreen_button': False,
    'use_issues_button': True,
    'use_repository_button': True,
    'use_source_button': False,
}
