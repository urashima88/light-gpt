import ast
import sys
from pathlib import Path
import json
import re
import importlib.util
import inspect
from typing import List, Optional, Tuple, Dict

root = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(root))

class ComponentContainerEditor:
    def __init__(self, file_path: str):
        self.file_path = file_path
        with open(file_path, 'r', encoding='utf-8') as f:
            self.source = f.read()
        self.source_lines = self.source.splitlines(keepends=True)
        self.tree = ast.parse(self.source)

    def analyze_class(self, class_name: str) -> dict:
        analyzer = _ClassAnalyzer(self.tree, class_name)
        result = analyzer.analyze()
        result['imports'] = self._collect_imports()
        return result

    def apply_changes(self, class_name: str, changes: dict) -> dict:
        if 'add_layer' in changes:
            self._add_layer(class_name, changes['add_layer'])
        if 'remove_layer' in changes:
            self._remove_layer(class_name, changes['remove_layer'])
        if 'update_layer_params' in changes:
            self._update_layer_params(class_name, changes['update_layer_params'])
        if 'set_forward_connections' in changes:
            self._set_forward_connections(class_name, changes['set_forward_connections'])
        if 'ensure_imports' in changes:
            self._ensure_imports(changes['ensure_imports'])

        new_source = ''.join(self.source_lines)
        with open(self.file_path, 'w', encoding='utf-8') as f:
            f.write(new_source)

        self.source = new_source
        self.tree = ast.parse(new_source)
        return self.analyze_class(class_name)

    def _collect_imports(self) -> List[Dict]:
        imports = []
        for node in ast.iter_child_nodes(self.tree):
            if isinstance(node, ast.Import):
                for alias in node.names:
                    imports.append({'module': alias.name, 'alias': alias.asname})
            elif isinstance(node, ast.ImportFrom):
                for alias in node.names:
                    full = (node.module + '.' + alias.name) if node.module else alias.name
                    imports.append({'module': full, 'alias': alias.asname})
        return imports

    def _add_layer(self, class_name: str, info: Dict):
        var = info['var']
        type_str = info['type']
        kwargs = info.get('kwargs', {})
        args_str = ', '.join(f"{k}={repr(v)}" for k, v in kwargs.items())
        line = f"        self.{var} = {type_str}({args_str})"

        init_start, init_end = self._find_method_body(class_name, '__init__')
        if init_start is None:
            class_line = self._find_class_line(class_name)
            self.source_lines.insert(class_line + 1, '    def __init__(self):\n')
            self.source_lines.insert(class_line + 2, '        super().__init__()\n')
            self.source_lines.insert(class_line + 3, line + '\n')
            return

        insert_idx = init_end
        for i in range(init_start, init_end):
            if 'super().__init__' in self.source_lines[i]:
                insert_idx = i + 1
                break

        self.source_lines.insert(insert_idx, line + '\n')

    def _remove_layer(self, class_name: str, var: str):
        pattern = re.compile(rf'\s*self\.{re.escape(var)}\s*=')
        self._delete_lines_matching(class_name, '__init__', pattern)

    def _update_layer_params(self, class_name: str, info: Dict):
        var = info['var']
        new_kwargs = info['kwargs']
        init_start, init_end = self._find_method_body(class_name, '__init__')
        if init_start is None:
            return
        pattern = re.compile(rf'(\s*self\.{re.escape(var)}\s*=\s*)(.*?)(\(.*\))')
        for i in range(init_start, init_end):
            match = pattern.match(self.source_lines[i])
            if match:
                prefix = match.group(1)
                type_part = match.group(2).strip()
                args_str = ', '.join(f"{k}={repr(v)}" for k, v in new_kwargs.items())
                new_line = f"{prefix}{type_part}({args_str})\n"
                self.source_lines[i] = new_line
                break

    def _set_forward_connections(self, class_name: str, connections: List[Dict]):
        order = self._linearize_connections(connections)
        input_var = self._get_forward_input_name(class_name)
        new_lines = []
        current = input_var
        for var in order:
            new_line = f"        {current} = self.{var}({current})"
            new_lines.append(new_line)
        return_line = f"        return {current}"

        self._replace_method_body(class_name, 'forward', new_lines, return_line)

    def _ensure_imports(self, imports: List[Dict]):
        for imp in imports:
            module = imp['module']
            alias = imp.get('alias')
            if not self._import_exists(module, alias):
                self._add_import(module, alias)

    def _find_method_body(self, class_name: str, method_name: str) -> Tuple[Optional[int], Optional[int]]:
        """
        Returns (start_line, end_line) of method body inside a class.
        """
        class_start = self._find_class_line(class_name)
        if class_start is None:
            return None, None

        in_class = False
        indent_level = None
        for i, line in enumerate(self.source_lines):
            stripped = line.strip()
            if i == class_start:
                in_class = True
                indent_level = len(line) - len(line.lstrip())
                continue
            if in_class:
                if stripped.startswith('class ') or (stripped.startswith('def ') and len(line) - len(line.lstrip()) <= indent_level):
                    break
                if stripped.startswith(f'def {method_name}('):
                    method_start = i
                    j = i + 1
                    while j < len(self.source_lines):
                        if self.source_lines[j].strip() == '':
                            j += 1
                            continue
                        current_indent = len(self.source_lines[j]) - len(self.source_lines[j].lstrip())
                        if current_indent <= indent_level:
                            break
                        j += 1
                    return method_start, j
        return None, None

    def _find_class_line(self, class_name: str) -> Optional[int]:
        """
        Returns index of the string with 'class ClassName(...)'.

        """
        for i, line in enumerate(self.source_lines):
            if re.match(rf'class\s+{re.escape(class_name)}\s*[:(]', line):
                return i
        return None

    def _delete_lines_matching(self, class_name, method_name, pattern):
        start, end = self._find_method_body(class_name, method_name)
        if start is None:
            return
        new_body = [line for line in self.source_lines[start:end] if not pattern.match(line)]
        self.source_lines[start:end] = new_body

    def _get_forward_input_name(self, class_name: str) -> str:
        start, _ = self._find_method_body(class_name, 'forward')
        if start is None:
            return 'x'
        line = self.source_lines[start]
        match = re.match(r'\s*def forward\(self,\s*(\w+)', line)
        if match:
            return match.group(1)
        return 'x'

    def _replace_method_body(self, class_name, method_name, body_lines, return_line):
        start, end = self._find_method_body(class_name, method_name)
        if start is None:
            class_line = self._find_class_line(class_name)
            indent = '    '
            new_method = [f'{indent}def {method_name}(self, x):\n'] + [f'{indent}    {l}\n' for l in body_lines] + [f'{indent}    {return_line}\n']
            self.source_lines[class_line+1:class_line+1] = new_method
            return
        sig_line = self.source_lines[start]
        indent = sig_line[:len(sig_line) - len(sig_line.lstrip())]
        new_body = [sig_line] + [f"{indent}    {l}\n" for l in body_lines] + [f"{indent}    {return_line}\n"]
        self.source_lines[start:end] = new_body

    def _linearize_connections(self, connections: List[Dict]) -> List[str]:
        graph = {}
        for conn in connections:
            frm = conn['from']
            to = conn['to']
            if frm not in graph:
                graph[frm] = []
            graph[frm].append(to)

        start = None
        all_nodes = set()
        for conn in connections:
            all_nodes.add(conn['from'])
            all_nodes.add(conn['to'])
        for node in all_nodes:
            if node not in [c['to'] for c in connections]:
                start = node
                break
        order = []
        visited = set()
        current = start
        while current is not None and current != 'output' and current not in visited:
            visited.add(current)
            if current != 'input':
                order.append(current)
            if current in graph and graph[current]:
                current = graph[current][0]
            else:
                break
        return order

    def _import_exists(self, module: str, alias: Optional[str]) -> bool:
        for imp in self._collect_imports():
            if imp['module'] == module and imp['alias'] == alias:
                return True
        return False

    def _add_import(self, module: str, alias: Optional[str]):
        line = f"import {module} as {alias}\n" if alias else f"import {module}\n"
        last_import = 0
        for i, l in enumerate(self.source_lines):
            if l.strip().startswith('import ') or l.strip().startswith('from '):
                last_import = i
        self.source_lines.insert(last_import + 1, line)

class _ClassAnalyzer(ast.NodeVisitor):
    def __init__(self, tree: ast.AST, class_name: str):
        self.tree = tree
        self.class_name = class_name
        self.layers = []
        self.forward_operations = []
        self.forward_input = None
        self.forward_return = None

    def analyze(self) -> dict:
        class_node = None
        for node in ast.walk(self.tree):
            if isinstance(node, ast.ClassDef) and node.name == self.class_name:
                class_node = node
                break
        if class_node is None:
            raise ValueError(f"Class {self.class_name} not found")

        for item in class_node.body:
            if isinstance(item, ast.FunctionDef) and item.name == '__init__':
                self._process_init(item)
            elif isinstance(item, ast.FunctionDef) and item.name == 'forward':
                self._process_forward(item)

        return {
            'layers': self.layers,
            'forward': {
                'input': self.forward_input,
                'operations': self.forward_operations,
                'return': self.forward_return
            }
        }

    def _process_init(self, node):
        for stmt in node.body:
            if isinstance(stmt, ast.Assign):
                for target in stmt.targets:
                    if isinstance(target, ast.Attribute) and isinstance(target.value, ast.Name) and target.value.id == 'self':
                        var = target.attr
                        value = stmt.value
                        if isinstance(value, ast.Call):
                            type_expr = ast.unparse(value.func) if hasattr(ast, 'unparse') else self._dump(value.func)
                            kwargs = {}
                            for kw in value.keywords:
                                kwargs[kw.arg] = ast.literal_eval(kw.value) if isinstance(kw.value, ast.Constant) else str(ast.dump(kw.value))
                            self.layers.append({'var': var, 'type': type_expr, 'kwargs': kwargs, 'line': stmt.lineno})

    def _process_forward(self, node):
        args = node.args.args
        if len(args) > 1:
            self.forward_input = args[1].arg
        for stmt in node.body:
            if isinstance(stmt, ast.Assign) and len(stmt.targets) == 1:
                target = stmt.targets[0]
                if isinstance(target, ast.Name):
                    out_var = target.id
                    value = stmt.value
                    if isinstance(value, ast.Call) and isinstance(value.func, ast.Attribute) and isinstance(value.func.value, ast.Name) and value.func.value.id == 'self':
                        layer_name = value.func.attr
                        in_var = None
                        if len(value.args) == 1 and isinstance(value.args[0], ast.Name):
                            in_var = value.args[0].id
                        self.forward_operations.append({'layer': layer_name, 'input': in_var, 'output': out_var})
            elif isinstance(stmt, ast.Return) and stmt.value and isinstance(stmt.value, ast.Name):
                self.forward_return = stmt.value.id

    def _dump(self, node):
        return ast.dump(node)

def get_constructor_params(cls):
    init_method = cls.__init__
    signature = inspect.signature(init_method)
    params = []
    for name, param in signature.parameters.items():
        if name == "self":
            continue
        has_default = param.default is not inspect.Parameter.empty
        default = repr(param.default) if has_default else None
        annotation = str(param.annotation) if param.annotation is not inspect.Parameter.empty else None
        params.append({"name": name, "type": annotation, "default": default, "has_default": has_default})
    return params

def get_forward_io(cls):
    forward = cls.forward
    signature = inspect.signature(forward)
    inputs = []
    for name, param in signature.parameters.items():
        if name == "self":
            continue
        annotation = str(param.annotation) if param.annotation is not inspect.Parameter.empty else None
        inputs.append({"name": name, "type": annotation})
    return_annotation = signature.return_annotation
    outputs = []
    if return_annotation is not inspect.Signature.empty:
        outputs = [{"name": "output", "type": str(return_annotation)}]
    else:
        outputs = [{"name": "output", "type": None}]
    return inputs, outputs

def inspect_class(file_path: str, class_name: str) -> dict:
    try:
        spec = importlib.util.spec_from_file_location("component_module", file_path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        cls = getattr(module, class_name)
        if not inspect.isclass(cls):
            raise ValueError(f"{class_name} is not a class")
        constructor_params = get_constructor_params(cls)
        inputs, outputs = get_forward_io(cls)
        return {
            "class_name": class_name,
            "file_path": file_path,
            "constructor_params": constructor_params,
            "inputs": inputs,
            "outputs": outputs
        }
    except Exception as e:
        return {"error": str(e)}

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"error": "Usage: component_code_editor.py <command> <args...>"}))
        sys.exit(1)

    command = sys.argv[1]

    if command == "analyze":
        if len(sys.argv) != 4:
            print(json.dumps({"error": "analyze <class_name> <file_path>"}))
            sys.exit(1)
        class_name = sys.argv[2]
        file_path = sys.argv[3]
        editor = ComponentContainerEditor(file_path)
        try:
            result = editor.analyze_class(class_name)
            print(json.dumps(result, indent=2))
        except Exception as e:
            print(json.dumps({"error": str(e)}))

    elif command == "apply_changes":
        if len(sys.argv) != 5:
            print(json.dumps({"error": "apply_changes <class_name> <file_path> <changes_json>"}))
            sys.exit(1)
        class_name = sys.argv[2]
        file_path = sys.argv[3]
        changes_json = sys.argv[4]
        try:
            changes = json.loads(changes_json)
        except json.JSONDecodeError as e:
            print(json.dumps({"error": f"Invalid JSON: {e}"}))
            sys.exit(1)
        editor = ComponentContainerEditor(file_path)
        try:
            updated = editor.apply_changes(class_name, changes)
            print(json.dumps(updated, indent=2))
        except Exception as e:
            print(json.dumps({"error": str(e)}))

    elif command == "inspect":
        if len(sys.argv) != 4:
            print(json.dumps({"error": "inspect <file_path> <class_name>"}))
            sys.exit(1)
        file_path = sys.argv[2]
        class_name = sys.argv[3]
        result = inspect_class(file_path, class_name)
        print(json.dumps(result, indent=2))

    else:
        print(json.dumps({"error": f"Unknown command: {command}"}))