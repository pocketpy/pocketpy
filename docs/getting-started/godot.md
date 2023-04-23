---
icon: code
label: Godot Engine
order: 2
---

## Introduction

PocketPy for Godot is integrated via [GDExtension](https://godotengine.org/article/introducing-gd-extensions).

!!!
GDExtension is a Godot 4.0 feature. Godot 3.x won't work.
!!!

Please see https://github.com/blueloveTH/godot-cpp/tree/master/pocketpy for details.

## Example

```gdscript
# main.gd

extends Node

func _ready():
	# Create a virtual machine
	var vm = pkpy.new_vm(false)

	# Run a script
	pkpy.vm_exec(vm, "print('Hello World!')")

	# Read the output
	var _o = pkpy.vm_read_output(vm)

	# Parse the output
	var res = JSON.parse_string(_o)

	# Print the output
	print(res["stdout"])    # "Hello World!\n"
```