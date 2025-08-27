import lldb
import pathlib

name = ""
known_modules = {(str(x).split("/")[-1][:-4]):x for x in pathlib.Path("./out/").glob("**/**.iko")}
module = None

def set_name_hook(frame: lldb.SBFrame, bp_loc: lldb.SBBreakpointLocation, internal_dict: dict):
    global known_modules
    global module
    global name
    # print("\nset_name_hook")
    process = frame.thread.process
    target = process.target
    debugger = target.debugger
    name = process.ReadCStringFromMemory(int(frame.args[0].value, 0), 128, lldb.SBError())
    # print("module path:", known_modules[name])
    module = target.AddModule(str(known_modules[name]), target.triple, "")
    return False

def load_module_hook(frame: lldb.SBFrame, bp_loc: lldb.SBBreakpointLocation, internal_dict: dict):
    global name
    # print("\nload_module_hook")
    # print(frame.args)
    file_offs = int(frame.args[0].value, 0)
    virt_addr = int(frame.args[1].value, 0)
    process = frame.thread.process
    target = process.target
    # find section
    section = None
    for sect in module.sections:
        if sect.file_offset == file_offs:
            section = sect
            break
    if section is None:
        raise RuntimeError("couldn't find section for " + name + " at " + str(file_offs))
    target.SetSectionLoadAddress(section, virt_addr)
    # print(section)
    return False
