import sys
import json
import importlib.util
import inspect

def get_constructor_params(cls):
    init_method = cls.__init__
    signature = inspect.signature(init_method)
    params = []
    for name, param in signature.parameters.items():
        if name == "self":
            continue
        has_default = param.default is not inspect.Parameter.empty
        default = repr(param.default) if has_default else None
        annotation = None
        if param.annotation is not inspect.Parameter.empty:
            annotation = str(param.annotation)
        params.append({
            "name": name,
            "type": annotation,
            "default": default,
            "has_default": has_default
        })
    return params

def get_forward_io(cls):
    forward = cls.forward
    signature = inspect.signature(forward)
    inputs = []
    for name, param in signature.parameters.items():
        if name == "self":
            continue
        annotation = None
        if param.annotation is not inspect.Parameter.empty:
            annotation = str(param.annotation)
        inputs.append({
            "name": name,
            "type": annotation
        })
    return_annotation = signature.return_annotation
    outputs = []
    if return_annotation is not inspect.Signature.empty:
        outputs = [{"name": "output", "type": str(return_annotation)}]
    else:
        outputs = [{"name": "output", "type": None}]
    return inputs, outputs

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(json.dumps({"error": "Usage: component_inspector.py <module_path> <class_name>"}))
        sys.exit(1)

    module_path = sys.argv[1]
    class_name = sys.argv[2]

    try:
        spec = importlib.util.spec_from_file_location("component_module", module_path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        cls = getattr(module, class_name)
        if not inspect.isclass(cls):
            raise ValueError(f"{class_name} is not a class")

        constructor_params = get_constructor_params(cls)
        inputs, outputs = get_forward_io(cls)

        result = {
            "class_name": class_name,
            "module_path": module_path,
            "constructor_params": constructor_params,
            "inputs": inputs,
            "outputs": outputs
        }
        print(json.dumps(result, indent=2))
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)
