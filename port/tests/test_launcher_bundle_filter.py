"""Exercise the actual spec filter without importing or running PyInstaller."""
import ast
from pathlib import Path
from types import SimpleNamespace
import unittest

from paths import ROOT


class LauncherBundleFilterTests(unittest.TestCase):
    @staticmethod
    def filter(platform):
        tree = ast.parse((ROOT / 'packaging/launcher.spec').read_text(encoding='utf-8'))
        nodes = [node for node in tree.body
                 if (isinstance(node, ast.FunctionDef) and node.name == 'keep') or
                 (isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and
                  target.id in ('drop', 'keep_anyway') for target in node.targets))]
        namespace = {'os': SimpleNamespace(name=platform)}
        exec(compile(ast.Module(body=nodes, type_ignores=[]), 'launcher.spec', 'exec'), namespace)
        return lambda path: namespace['keep']((path, '/unused/source', 'BINARY'))

    def test_windows_icu_excluded_at_any_bundle_location(self):
        keep = self.filter('nt')
        for path in ('icuuc.dll', 'PySide6/icuuc.dll', 'PySide6\\ICUUC.DLL'):
            self.assertFalse(keep(path), path)
        self.assertTrue(keep('PySide6/Qt6Core.dll'))
        self.assertTrue(keep('PySide6/QtCore.pyd'))

    def test_linux_icu_libraries_retained(self):
        keep = self.filter('posix')
        for path in ('libicuuc.so.78', 'PySide6/Qt/lib/libicuuc.so.78', 'icuuc.dll'):
            self.assertTrue(keep(path), path)

    def test_existing_plugin_rules_unchanged(self):
        for platform in ('nt', 'posix'):
            keep = self.filter(platform)
            self.assertFalse(keep('PySide6/Qt/plugins/qmltooling/plugin.dll'))
            self.assertTrue(keep('PySide6/Qt/qml/Qt/labs/folderlistmodel/plugin.dll'))


if __name__ == '__main__':
    unittest.main()
