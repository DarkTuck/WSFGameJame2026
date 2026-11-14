"""Unreal side Pixel Art Studio for Blender
"""

import hashlib
import json
import os
import re
import sys
import time

import unreal

PLUGIN = "PixelArtStudioBlender"
HERE = os.path.dirname(os.path.abspath(__file__))
PLUGIN_ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
UPLUGIN = os.path.join(PLUGIN_ROOT, PLUGIN + ".uplugin")
MANIFEST = os.path.join(PLUGIN_ROOT, "Data", "pixel-art-studio.json")
MAT_DIR = "/Game/PixelArtStudioBlender/Materials"
PARAM = "Canvas"
TAG = "PixelArtStudio"
FORMAT = 1
WATCH_EVERY = 2.0
MENU = "LevelEditor.MainMenu.Tools"
SECTION = "PixelArtStudio"
REFUSED = ("content", "saved", "intermediate", "deriveddatacache",
           "binaries", "plugins")
TRACE = os.environ.get("PAS_TRACE") == "1"

_state = {"started": False, "busy": False, "tick": None, "menu": [],
          "next": 0.0, "stamp": None, "ready": False, "startup_done": False, "startup_begun": False,
          "shutdown": False, "later": set(), "in_tick": False,
          "import_seen": 0.0}
_cache = {}
_said = set()
STATS = {"callbacks": 0, "textures_fixed": 0, "meshes_fixed": 0,
         "slots_left": 0, "assets_loaded": 0, "imported": 0}


# ----------------------------------------------------------------
# Logging
# ----------------------------------------------------------------

def log(msg):
    unreal.log("Pixel Art Studio: " + msg)


def once(kind, msg):
    if kind in _said:
        return
    _said.add(kind)
    unreal.log_warning("Pixel Art Studio: " + msg)


def trace(msg):
    if TRACE:
        unreal.log("PASTRACE " + msg)


# ----------------------------------------------------------------
# The version: the `.uplugin`'s `VersionName`, nowhere else
# ----------------------------------------------------------------

def version():
    if "version" not in _cache:
        try:
            with open(UPLUGIN, encoding="utf8") as fh:
                _cache["version"] = str(json.load(fh).get("VersionName", ""))
        except (OSError, ValueError):
            _cache["version"] = ""
    return _cache["version"]


# ----------------------------------------------------------------
# Paths
# ----------------------------------------------------------------

def _norm(path):
    return os.path.normcase(os.path.normpath(path))


def project_dir():
    if "project" not in _cache:
        _cache["project"] = os.path.normpath(
            unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
    return _cache["project"]


def state_path():
    return os.path.join(project_dir(), "Saved", PLUGIN, "state.json")


def project_file(rel):
    if not isinstance(rel, str) or not rel or os.path.isabs(rel) \
            or rel.startswith(("/", "\\")) or re.match(r"^[A-Za-z]:", rel):
        return None
    parts = [p for p in re.split(r"[\\/]+", rel) if p not in ("", ".")]
    if not parts or ".." in parts or parts[0].lower() in REFUSED:
        return None
    return os.path.normpath(os.path.join(project_dir(), *parts))


BAD_DEST = set("\\:*?\"<>|' ,.&!~\n\r\t@#")


def _dest(value):
    if not isinstance(value, str):
        return None
    value = value.rstrip("/")
    if not value.startswith("/Game/") or "//" in value \
            or BAD_DEST.intersection(value):
        return None
    return value


def _write_json(path, doc):
    data = (json.dumps(doc, indent=1, sort_keys=True, ensure_ascii=False)
            + "\n").encode("utf8")
    try:
        with open(path, "rb") as fh:
            if fh.read() == data:
                return
    except OSError:
        pass
    os.makedirs(os.path.dirname(path), exist_ok=True)
    tmp = path + "tmp"
    with open(tmp, "wb") as fh:
        fh.write(data)
    for attempt in range(8):
        try:
            os.replace(tmp, path)
            return
        except PermissionError:
            time.sleep(0.05 * (attempt + 1))
    os.remove(tmp)


# ----------------------------------------------------------------
# The manifest
# ----------------------------------------------------------------

EMPTY = {"watch": True, "models": {}, "materials": {}, "sha1": "",
         "files": {}}


def manifest():
    try:
        st = os.stat(MANIFEST)
    except OSError:
        return EMPTY
    key = (st.st_mtime, st.st_size)
    if _cache.get("manifest_key") == key:
        return _cache["manifest"]
    try:
        with open(MANIFEST, "rb") as fh:
            raw = fh.read()
        doc = json.loads(raw.decode("utf8"))
    except (OSError, ValueError) as ex:
        once("manifest-json", "the manifest could not be read (%s); "
             "nothing is fixed until Blender writes it again" % ex)
        doc, raw = None, b""
    try:
        view = _read_manifest(doc, raw)
    except Exception as ex:
        once("manifest-shape", "the manifest is not in the shape this "
             "plugin reads (%s); nothing is fixed until Blender writes it "
             "again" % ex)
        view = EMPTY
    _cache["manifest_key"] = key
    _cache["manifest"] = view
    return view


def _read_manifest(doc, raw):
    if not isinstance(doc, dict):
        return EMPTY
    if doc.get("format") != FORMAT:
        once("manifest-format", "the manifest is format %s and this plugin "
             "reads format %d; update the plugin from Blender"
             % (doc.get("format"), FORMAT))
        return EMPTY
    view = {"watch": doc.get("watch") is not False, "models": {},
            "materials": {}, "sha1": hashlib.sha1(raw).hexdigest(),
            "files": {}}
    top = doc.get("materials")
    if isinstance(top, dict):
        view["materials"] = {str(k): str(v) for k, v in top.items()
                             if isinstance(v, str)}
    models = doc.get("models")
    for rel, entry in (models.items() if isinstance(models, dict) else ()):
        path = project_file(rel)
        dest = _dest(entry.get("dest")) if isinstance(entry, dict) else None
        if path is None or dest is None:
            once("manifest-path", "left out a model the manifest names where "
                 "it may not look: %s" % rel)
            continue
        mats = entry.get("materials")
        canvases = {}
        found = entry.get("canvases")
        for stem, info in (found.items() if isinstance(found, dict) else ()):
            if not isinstance(info, dict):
                continue
            file_rel = info.get("file") or ""
            file_path = project_file(file_rel) if file_rel else None
            if file_rel and file_path is None:
                once("manifest-path", "left out a canvas the manifest names "
                     "where it may not look: %s" % file_rel)
            cut = info.get("cut")
            canvases[str(stem)] = {"cut": cut if isinstance(cut, bool) else None,
                                   "path": file_path}
            if file_path:
                view["files"][_norm(file_path)] = {
                    "path": file_path, "dest": dest, "model": False}
        view["models"][_norm(path)] = {
            "path": path, "dest": dest,
            "materials": {str(k): str(v) for k, v in
                          (mats.items() if isinstance(mats, dict) else ())},
            "canvases": canvases}
        view["files"][_norm(path)] = {"path": path, "dest": dest,
                                      "model": True}
    return view


# ----------------------------------------------------------------
# The user's own state, and the watch that runs
# ----------------------------------------------------------------

def read_state():
    try:
        with open(state_path(), encoding="utf8") as fh:
            got = json.load(fh)
        return got if isinstance(got, dict) else {}
    except (OSError, ValueError):
        return {}


def write_state(**changes):
    doc = read_state()
    doc.update(changes)
    for key in ("fixed", "plugin", "watch"):
        doc.setdefault(key, None if key == "watch" else "")
    _write_json(state_path(), doc)


def watch_on():
    user = read_state().get("watch")
    if isinstance(user, bool):
        return user
    return manifest()["watch"]


def set_user_watch(on):
    team = manifest()["watch"]
    write_state(watch=None if bool(on) == team else bool(on))
    log("updating from Blender automatically is %s for you"
        % ("on" if watch_on() else "off"))


def toggle_watch():
    set_user_watch(not watch_on())
    _refresh_menu()


# ----------------------------------------------------------------
# Where an asset came from
# ----------------------------------------------------------------

def _tag_entries(data):
    try:
        raw = data.get_tag_value("AssetImportData")
    except Exception:
        return []
    if not raw:
        return []
    try:
        got = json.loads(str(raw))
    except ValueError:
        return []
    return got if isinstance(got, list) else []


def _package_dir(package_path):
    path = str(package_path)
    if not path.startswith("/Game/"):
        return None
    folder = path[len("/Game/"):].rsplit("/", 1)[0] if "/" in path[6:] else ""
    return os.path.normpath(os.path.join(project_dir(), "Content", folder))


def resolve(rel, package_path):
    rel = str(rel or "")
    if not rel:
        return []
    if os.path.isabs(rel):
        return [_norm(rel)]
    out = []
    base = _package_dir(package_path)
    if base:
        out.append(_norm(os.path.join(base, rel)))
    try:
        out.append(_norm(unreal.Paths.convert_relative_path_to_full(rel)))
    except Exception:
        pass
    return out


def data_sources(data):
    out = []
    package = str(data.package_name)
    for entry in _tag_entries(data):
        out.append((resolve(entry.get("RelativeFilename", ""), package),
                    entry.get("FileMD5", "")))
    return out


def obj_sources(obj):
    try:
        data = obj.get_editor_property("asset_import_data")
    except Exception:
        return []
    if data is None:
        return []
    try:
        return [_norm(str(f)) for f in data.extract_filenames()]
    except Exception:
        return []


def _entry_of(paths):
    models = manifest()["models"]
    for p in paths:
        if p in models:
            return models[p]
    return None


def _canvas_file_stem(paths):
    for entry in manifest()["models"].values():
        for stem, info in entry["canvases"].items():
            if info["path"] and _norm(info["path"]) in paths:
                return stem, entry
    return None, None


def _bare(name):
    return re.sub(r"^\d+_", "", str(name))


# ----------------------------------------------------------------
# Textures
# ----------------------------------------------------------------

TEXTURE_SETTINGS = (
    ("filter", "TextureFilter", "TF_NEAREST"),
    ("mip_gen_settings", "TextureMipGenSettings", "TMGS_NO_MIPMAPS"),
    ("compression_settings", "TextureCompressionSettings", "TC_EDITOR_ICON"),
    ("lod_group", "TextureGroup", "TEXTUREGROUP_PIXELS2D"),
    ("srgb", None, True),
    ("never_stream", None, True),
)


def texture_stem(tex, paths=None):
    paths = obj_sources(tex) if paths is None else paths
    stem, _entry = _canvas_file_stem(paths)
    if stem:
        return stem
    entry = _entry_of(paths)
    if entry is not None:
        return stem_named(tex.get_name(), entry["canvases"])
    return None


def stem_named(asset_name, canvases):
    name = _bare(asset_name)
    if name in canvases:
        return name
    want = loose(name)
    for stem in sorted(canvases):
        if loose(stem) == want:
            return stem
    return None


def fix_texture(tex):
    changed = []
    for name, enum, value in TEXTURE_SETTINGS:
        want = getattr(getattr(unreal, enum), value) if enum else value
        try:
            if tex.get_editor_property(name) != want:
                tex.set_editor_property(name, want)
                changed.append(name)
        except Exception as ex:
            once("texture-" + name, "could not set %s: %s" % (name, ex))
    if changed:
        STATS["textures_fixed"] += 1
        save(tex)
        trace("texture %s fixed %s" % (tex.get_path_name(), changed))
    return changed


def _registry():
    return unreal.AssetRegistryHelpers.get_asset_registry()


def _assets_of(cls):
    reg = _registry()
    try:
        return reg.get_assets_by_class(cls, True)
    except TypeError:
        return reg.get_assets_by_class(
            unreal.TopLevelAssetPath("/Script/Engine", cls), True)


def _class_name(data):
    for attr in ("asset_class_path", "asset_class"):
        try:
            value = getattr(data, attr)
        except AttributeError:
            continue
        text = str(value.get_editor_property("asset_name")) \
            if hasattr(value, "get_editor_property") else str(value)
        return text.rsplit(".", 1)[-1]
    return ""


def find_texture(stem, mesh, entry):
    folder = mesh.get_path_name().rsplit("/", 1)[0]
    folders = [folder]
    if folder.rsplit("/", 1)[-1] in ("StaticMeshes", "SkeletalMeshes", "Meshes"):
        folders.append(folder.rsplit("/", 1)[0])
    reg = _registry()
    for where in folders:
        for data in reg.get_assets_by_path(where, recursive=True):
            if stem_named(data.asset_name, (stem,)) is None:
                continue
            obj = data.get_asset()
            if isinstance(obj, unreal.Texture2D):
                return obj
    info = entry["canvases"].get(stem) if entry else None
    if info and info["path"]:
        want = _norm(info["path"])
        for data in _assets_of("Texture2D"):
            if any(want in cands for cands, _md5 in data_sources(data)):
                obj = data.get_asset()
                if isinstance(obj, unreal.Texture2D):
                    return obj
    return None


# ----------------------------------------------------------------
# Materials
# ----------------------------------------------------------------

def _tools():
    return unreal.AssetToolsHelpers.get_asset_tools()


def _asset_lib():
    try:
        return unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    except Exception:
        return unreal.EditorAssetLibrary


def _exists(path):
    return _asset_lib().does_asset_exist(path)


def tag(obj):
    try:
        _asset_lib().set_metadata_tag(obj, TAG, version())
    except Exception:
        pass


def is_ours(mat):
    if mat is None:
        return False
    if mat.get_path_name().startswith(MAT_DIR + "/"):
        return True
    try:
        return bool(_asset_lib().get_metadata_tag(mat, TAG))
    except Exception:
        return False


def parent_material(cut):
    name = "M_PixelArtStudio_Cut" if cut else "M_PixelArtStudio_Opaque"
    path = MAT_DIR + "/" + name
    if _exists(path):
        return unreal.load_asset(path)
    lib = unreal.MaterialEditingLibrary
    mat = _tools().create_asset(name, MAT_DIR, unreal.Material,
                                unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model",
                            unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED
                            if cut else unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("two_sided", True)
    if cut:
        mat.set_editor_property("opacity_mask_clip_value", 0.5)
    tex = lib.create_material_expression(
        mat, unreal.MaterialExpressionTextureSampleParameter2D, -500, 0)
    tex.set_editor_property("parameter_name", PARAM)
    tex.set_editor_property("texture", unreal.load_asset(
        "/Engine/EngineResources/DefaultTexture"))
    lib.connect_material_property(tex, "RGB",
                                  unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if cut:
        lib.connect_material_property(tex, "A",
                                      unreal.MaterialProperty.MP_OPACITY_MASK)
    try:
        lib.set_material_usage(mat, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
    except Exception:
        pass
    lib.recompile_material(mat)
    tag(mat)
    save(mat)
    return mat


def _asset_name(stem):
    return "MI_" + re.sub(r"[^A-Za-z0-9_]", "_", stem)


def material_instance(stem, texture, cut):
    parent = parent_material(cut)
    folder = texture.get_path_name().rsplit("/", 1)[0]
    name = _asset_name(stem)
    path = folder + "/" + name
    if _exists(path):
        mi = unreal.load_asset(path)
    else:
        mi = _tools().create_asset(name, folder,
                                   unreal.MaterialInstanceConstant,
                                   unreal.MaterialInstanceConstantFactoryNew())
        tag(mi)
    lib = unreal.MaterialEditingLibrary
    dirty = False
    if mi.get_editor_property("parent") != parent:
        lib.set_material_instance_parent(mi, parent)
        dirty = True
    if lib.get_material_instance_texture_parameter_value(mi, PARAM) != texture:
        lib.set_material_instance_texture_parameter_value(mi, PARAM, texture)
        dirty = True
    if dirty:
        lib.update_material_instance(mi)
        save(mi)
    return mi


def _cut_for(stem, entry):
    info = entry["canvases"].get(stem) if entry else None
    if info is None:
        for other in manifest()["models"].values():
            if stem in other["canvases"]:
                info = other["canvases"][stem]
                break
    if info is not None and info["cut"] is not None:
        return info["cut"]
    return True


# ----------------------------------------------------------------
# Meshes
# ----------------------------------------------------------------

def loose(name):
    out = []
    for byte in str(name).encode("utf8"):
        ch = chr(byte)
        out.append(ch if byte < 128 and ch.isalnum() else "_")
    return re.sub(r"_+", "_", "".join(out))


def _slot_stem(entry, slot_name, current):
    maps = ([entry["materials"]] if entry is not None else []) \
        + [manifest()["materials"]]
    for name in (slot_name, current.get_name() if current is not None else ""):
        if not name:
            continue
        for mats in maps:
            if name in mats:
                return name, mats[name]
        want = loose(name)
        for mats in maps:
            for key, stem in sorted(mats.items()):
                if loose(key) == want:
                    return key, stem
    if current is not None and is_ours(current):
        for mats in maps:
            for key, stem in sorted(mats.items()):
                if _asset_name(stem) == current.get_name():
                    return key, stem
    return None, None


def fix_mesh(sm, paths=None):
    paths = obj_sources(sm) if paths is None else paths
    entry = _entry_of(paths)
    slots = list(sm.get_editor_property("static_materials") or [])
    changed = False
    for i, slot in enumerate(slots):
        imported = str(slot.get_editor_property("imported_material_slot_name"))
        current = slot.get_editor_property("material_interface")
        name, stem = _slot_stem(entry, imported, current)
        if not stem:
            continue
        if current is not None and not is_ours(current) \
                and loose(current.get_name()) != loose(name):
            STATS["slots_left"] += 1
            once("slot-" + sm.get_path_name() + name,
                 "left the material %s on slot %s of %s alone: it is not "
                 "the one the import made"
                 % (current.get_name(), name, sm.get_name()))
            continue
        texture = find_texture(stem, sm, entry)
        if texture is None:
            once("notex-" + stem, "no texture %s for %s yet"
                 % (stem, sm.get_name()))
            continue
        fix_texture(texture)
        mi = material_instance(stem, texture, _cut_for(stem, entry))
        if current != mi:
            slot.set_editor_property("material_interface", mi)
            slots[i] = slot
            changed = True
    if changed:
        sm.set_editor_property("static_materials", slots)
        STATS["meshes_fixed"] += 1
        save(sm)
        trace("mesh %s fixed" % sm.get_path_name())
    return changed


# ----------------------------------------------------------------
# Saving
# ----------------------------------------------------------------

def save(obj):
    try:
        unreal.EditorLoadingAndSavingUtils.save_packages(
            [obj.get_outermost()], False)
    except Exception as ex:
        once("save", "could not save %s: %s" % (obj.get_name(), ex))


# ----------------------------------------------------------------
# At import
# ----------------------------------------------------------------

def on_post_import(factory, obj):
    STATS["callbacks"] += 1
    _state["import_seen"] = time.time()
    if _state["busy"] or obj is None:
        return
    _state["busy"] = True
    try:
        trace("post import %s %s" % (obj.get_class().get_name(),
                                     obj.get_path_name()))
        if isinstance(obj, unreal.Texture2D):
            if texture_stem(obj):
                fix_texture(obj)
                _state["later"].add(obj.get_path_name().rsplit("/", 1)[0])
        elif isinstance(obj, unreal.StaticMesh):
            fix_mesh(obj)
        elif isinstance(obj, unreal.Package):
            sweep(obj.get_name().rsplit("/", 1)[0])
    except Exception as ex:
        once(type(ex).__name__, "%s on %s: %s"
             % (type(ex).__name__, obj.get_name(), ex))
    finally:
        _state["busy"] = False


def _near(folder):
    out = [folder]
    if folder.rsplit("/", 1)[-1] == "Textures":
        parent = folder.rsplit("/", 1)[0]
        out += [parent, parent + "/StaticMeshes"]
    return out


def fix_waiting():
    folders, _state["later"] = _state["later"], set()
    reg = _registry()
    _state["busy"] = True
    try:
        for folder in sorted(folders):
            for where in _near(folder):
                for data in reg.get_assets_by_path(where, recursive=False):
                    if _class_name(data) != "StaticMesh":
                        continue
                    cands = set()
                    for paths, _md5 in data_sources(data):
                        cands.update(paths)
                    if _entry_of(cands) is None:
                        continue
                    fix_mesh(data.get_asset(), cands)
    finally:
        _state["busy"] = False


def on_pre_import(factory, cls, parent, name, kind):
    _state["import_seen"] = time.time()


def _importing():
    if time.time() - _state["import_seen"] < 1.0:
        return True
    try:
        manager = unreal.InterchangeManager.get_interchange_manager_scripted()
        return bool(manager.is_interchange_active())
    except Exception:
        return False


def sweep(folder):
    meshes = []
    for data in _registry().get_assets_by_path(folder, recursive=True):
        obj = data.get_asset()
        if isinstance(obj, unreal.Texture2D):
            if texture_stem(obj):
                fix_texture(obj)
        elif isinstance(obj, unreal.StaticMesh):
            meshes.append(obj)
    for obj in meshes:
        fix_mesh(obj)


# ----------------------------------------------------------------
# Fix Models in Project
# ----------------------------------------------------------------

def fix_project():
    reg = _registry()
    try:
        reg.search_all_assets(True)
    except Exception:
        pass
    found = {"meshes": 0, "textures": 0}
    folders = set()
    _state["busy"] = True
    try:
        for cls in ("Texture2D", "StaticMesh"):
            for data in _assets_of(cls):
                if not str(data.package_path).startswith("/Game"):
                    continue
                cands = set()
                for paths, _md5 in data_sources(data):
                    cands.update(paths)
                if not cands:
                    continue
                if cls == "Texture2D":
                    stem, _e = _canvas_file_stem(cands)
                    entry = _entry_of(cands)
                    if not stem and not (entry and stem_named(
                            data.asset_name, entry["canvases"])):
                        continue
                    obj = data.get_asset()
                    STATS["assets_loaded"] += 1
                    if fix_texture(obj):
                        found["textures"] += 1
                    if entry is not None:
                        folder = str(data.package_path)
                        if folder.rsplit("/", 1)[-1] == "Textures":
                            folder = folder.rsplit("/", 1)[0]
                        folders.add(folder)
                else:
                    if _entry_of(cands) is None:
                        continue
                    obj = data.get_asset()
                    STATS["assets_loaded"] += 1
                    if fix_mesh(obj, cands):
                        found["meshes"] += 1
        for folder in sorted(folders):
            for data in reg.get_assets_by_path(folder, recursive=True):
                if _class_name(data) != "StaticMesh" or data_sources(data):
                    continue
                obj = data.get_asset()
                STATS["assets_loaded"] += 1
                if fix_mesh(obj, _glb_paths_for(folder)):
                    found["meshes"] += 1
    finally:
        _state["busy"] = False
    log("fixed %d model(s) and %d texture(s)"
        % (found["meshes"], found["textures"]))
    return found


def _glb_paths_for(folder):
    out = []
    for data in _registry().get_assets_by_path(folder, recursive=True):
        for paths, _md5 in data_sources(data):
            for p in paths:
                if p in manifest()["models"]:
                    out.append(p)
    return out


# ----------------------------------------------------------------
# Bringing changes in
# ----------------------------------------------------------------

def _file_md5(path):
    st = os.stat(path)
    key = (st.st_mtime, st.st_size)
    got = _cache.get(("md5", path))
    if got is None or got[0] != key:
        with open(path, "rb") as fh:
            got = (key, hashlib.md5(fh.read()).hexdigest())
        _cache[("md5", path)] = got
    return got[1]


def imported(dest, path, md5):
    want = _norm(path)
    name = os.path.basename(want)
    by_path, by_name = [], []
    for data in _registry().get_assets_by_path(dest, recursive=True):
        for paths, got in data_sources(data):
            if want in paths:
                by_path.append(got)
            elif any(os.path.basename(p) == name for p in paths):
                by_name.append(got)
    return md5 in (by_path or by_name)


def pending():
    view = manifest()
    canvases, models = [], []
    for _norm_path, info in sorted(view["files"].items()):
        path, dest = info["path"], info["dest"]
        if not os.path.isfile(path):
            continue
        if not imported(dest, path, _file_md5(path)):
            (models if info["model"] else canvases).append((path, dest))
    return canvases + models


def import_pending():
    todo = pending()
    if not todo:
        return 0
    tasks = []
    for path, dest in todo:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", dest)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    _tools().import_asset_tasks(tasks)
    for path, dest in todo:
        refresh_embedded(path, dest)
    try:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(False, True)
    except Exception as ex:
        once("save-dirty", "could not save what the import made: %s" % ex)
    STATS["imported"] += len(tasks)
    log("brought in %d changed file(s) from Blender" % len(tasks))
    trace("imported %s" % [os.path.basename(p) for p, _d in todo])
    return len(tasks)


def refresh_embedded(path, dest):
    try:
        manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    except Exception:
        return 0
    want = _norm(path)
    md5 = _file_md5(path)
    done = 0
    for data in _registry().get_assets_by_path(dest, recursive=True):
        if _class_name(data) != "Texture2D":
            continue
        if not any(want in paths and got != md5
                   for paths, got in data_sources(data)):
            continue
        tex = data.get_asset()
        try:
            params = unreal.ImportAssetParameters()
            params.set_editor_property("reimport_asset", tex)
            params.set_editor_property("is_automated", True)
            source = unreal.InterchangeManager.create_source_data(path)
            manager.import_asset(str(data.package_path), source, params)
            done += 1
            trace("reimported %s out of %s" % (tex.get_path_name(),
                                                os.path.basename(path)))
        except Exception as ex:
            once("embedded", "could not bring the texture %s in again from "
                 "%s: %s" % (tex.get_name(), os.path.basename(path), ex))
    return done


def _stamp():
    manifest()
    parts = [_cache.get("manifest_key")]
    for norm in sorted(manifest()["files"]):
        try:
            st = os.stat(norm)
            parts.append((norm, st.st_mtime, st.st_size))
        except OSError:
            parts.append((norm, None))
    return tuple(parts)


def _playing():
    try:
        sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        return sub.get_game_world() is not None
    except Exception:
        try:
            return unreal.EditorLevelLibrary.get_game_world() is not None
        except Exception:
            return False


def refix():
    sha = manifest()["sha1"]
    if not sha or sha == _state.get("fixed_sha"):
        return
    _state["fixed_sha"] = sha
    fix_project()
    write_state(fixed=sha, plugin=version())


def _loading():
    try:
        return bool(_registry().is_loading_assets())
    except Exception:
        return False


def startup():
    _state["startup_begun"] = True
    try:
        try:
            _registry().search_all_assets(True)
        except Exception:
            pass
        import_pending()
        view = manifest()
        st = read_state()
        if view["sha1"] and (st.get("fixed") != view["sha1"]
                             or st.get("plugin") != version()):
            fix_project()
            write_state(fixed=view["sha1"], plugin=version())
        _state["fixed_sha"] = view["sha1"]
        _state["stamp"] = _stamp()
        trace("startup done watch=%s" % watch_on())
    except Exception as ex:
        once("startup-" + type(ex).__name__, "the start up pass failed: %s" % ex)
    finally:
        _state["startup_done"] = True


def _nested():
    try:
        sys._getframe(2)
        return True
    except ValueError:
        return False


def tick(_delta):
    if _state["shutdown"] or _state["in_tick"] or _nested():
        return
    _state["in_tick"] = True
    try:
        _tick()
    finally:
        _state["in_tick"] = False


def _tick():
    if _state["later"] and not _state["busy"] and not _importing():
        try:
            fix_waiting()
        except Exception as ex:
            once("waiting-" + type(ex).__name__,
                 "a model waiting for its canvas: %s" % ex)
    now = time.time()
    if now < _state["next"]:
        return
    _state["next"] = now + WATCH_EVERY
    try:
        if _importing():
            _state["next"] = 0.0
            return
        if not _state["startup_begun"]:
            if _loading():
                return
            startup()
            return
        if not watch_on() or _playing():
            return
        stamp = _stamp()
        if stamp == _state["stamp"]:
            return
        _state["stamp"] = stamp
        import_pending()
        refix()
    except Exception as ex:
        once("watch-" + type(ex).__name__, "the watch failed: %s" % ex)


# ----------------------------------------------------------------
# The Tools menu
# ----------------------------------------------------------------

FIX_LABEL = "Fix Pixel Art Studio Models"
FIX_TIP = ("Make every model and texture Pixel Art Studio exported sharp "
           "and unlit again")
WATCH_LABEL = "Update From Blender Automatically"
WATCH_TIP = ("Bring each save from Blender in a few seconds later. The "
             "project sets it for everyone; this changes it for you only")

_Toggle = None
if hasattr(unreal, "ToolMenuEntryScript"):
    try:
        @unreal.uclass()
        class PixelArtStudioBlenderWatchEntry(unreal.ToolMenuEntryScript):
            @unreal.ufunction(override=True)
            def execute(self, context):
                toggle_watch()

            @unreal.ufunction(override=True)
            def get_check_state(self, context):
                return unreal.CheckBoxState.CHECKED if watch_on() \
                    else unreal.CheckBoxState.UNCHECKED

        _Toggle = PixelArtStudioBlenderWatchEntry
    except Exception as ex:
        once("menu-class", "no switch entry: %s" % ex)


def _string_entry(name, label, tip, command):
    entry = unreal.ToolMenuEntry(name=name,
                                 type=unreal.MultiBlockType.MENU_ENTRY)
    entry.set_label(label)
    entry.set_tool_tip(tip)
    entry.set_string_command(unreal.ToolMenuStringCommandType.PYTHON, "",
                             command)
    return entry


def register_menu():
    try:
        menus = unreal.ToolMenus.get()
        menu = menus.extend_menu(MENU)
        try:
            menu.add_section(SECTION, "Pixel Art Studio")
        except Exception:
            pass
        menu.add_menu_entry(SECTION, _string_entry(
            "PixelArtStudioFix", FIX_LABEL, FIX_TIP,
            "import pixel_art_studio_blender as p; p.fix_project()"))
        if _Toggle is not None:
            entry = _Toggle()
            entry.init_entry(PLUGIN, MENU, SECTION, "PixelArtStudioWatch",
                             WATCH_LABEL, WATCH_TIP)
            data = entry.get_editor_property("data")
            adv = data.get_editor_property("advanced")
            adv.set_editor_property("user_interface_action_type",
                                    unreal.UserInterfaceActionType.TOGGLE_BUTTON)
            data.set_editor_property("advanced", adv)
            entry.set_editor_property("data", data)
            menu.add_menu_entry_object(entry)
            _state["menu"] = [entry]
        else:
            _add_fallback(menu)
        menus.refresh_all_widgets()
        return True
    except Exception as ex:
        once("menu", "no menu entry: %s" % ex)
        return False


def _add_fallback(menu):
    menu.add_menu_entry(SECTION, _string_entry(
        "PixelArtStudioWatch",
        WATCH_LABEL + (" (On)" if watch_on() else " (Off)"), WATCH_TIP,
        "import pixel_art_studio_blender as p; p.toggle_watch()"))


def _refresh_menu():
    if _Toggle is not None:
        return
    try:
        menus = unreal.ToolMenus.get()
        menus.remove_entry(MENU, SECTION, "PixelArtStudioWatch")
        _add_fallback(menus.extend_menu(MENU))
        menus.refresh_all_widgets()
    except Exception:
        pass


# ----------------------------------------------------------------
# Start and stop
# ----------------------------------------------------------------

def stop():
    _state["shutdown"] = True
    sub = _state.get("subsystem")
    if sub is not None:
        for event, fn in (("on_asset_pre_import", on_pre_import),
                          ("on_asset_post_import", on_post_import)):
            try:
                getattr(sub, event).remove_callable(fn)
            except Exception:
                pass
        _state["subsystem"] = None
    if _state.get("tick") is not None:
        try:
            unreal.unregister_slate_post_tick_callback(_state["tick"])
        except Exception:
            pass
        _state["tick"] = None
    try:
        menus = unreal.ToolMenus.get()
        for name in ("PixelArtStudioFix", "PixelArtStudioWatch"):
            menus.remove_entry(MENU, SECTION, name)
    except Exception:
        pass
    _state["menu"] = []


def start():
    if _state["started"]:
        return
    _state["started"] = True
    sub = unreal.get_editor_subsystem(unreal.ImportSubsystem)
    _state["subsystem"] = sub
    for event, fn in (("on_asset_pre_import", on_pre_import),
                      ("on_asset_post_import", on_post_import)):
        try:
            getattr(sub, event).add_callable(fn)
        except Exception as ex:
            once("bind-" + event, "could not listen to %s: %s" % (event, ex))
    menu = register_menu()
    _state["tick"] = unreal.register_slate_post_tick_callback(tick)
    try:
        unreal.register_python_shutdown_callback(stop)
    except Exception as ex:
        once("shutdown", "nothing unbinds at shutdown: %s" % ex)
    log("plugin %s started" % version())
    trace("started %s manifest=%s menu=%s toggle=%s watch=%s" % (
        version(), os.path.exists(MANIFEST), menu, _Toggle is not None,
        watch_on()))
