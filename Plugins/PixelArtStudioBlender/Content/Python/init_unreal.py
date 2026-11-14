try:
    import pixel_art_studio_blender
    pixel_art_studio_blender.start()
except Exception as ex:
    import unreal
    unreal.log_warning("Pixel Art Studio: could not start: %s" % ex)
