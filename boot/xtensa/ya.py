# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************

if has_config('single-kernel'): 
    target("boot-init.elf")
    set_kind("object")
    pass
else:
    target("boot-init.elf")
    ## set_extensions(".h",".o")
    add_deps('boot-config')
    add_cflags("-mlongcalls", "-mtext-section-literals", "-mabi=call0")
    add_asflags("-mlongcalls", "-mtext-section-literals", "-mabi=call0")
    add_ldflags("-mlongcalls", "-mtext-section-literals", "-mabi=call0", force = true)

    add_files(
        'boot.s',
        'vectors.s',
        'init.c'
    )
    add_includedirs(
        '.',
        '../../duck',
        '../../duck/platform/{plat}'
    )


    add_ldflags("-T"+path.join(os.scriptdir(), "../xtensa/link.ld"), force = true)

    add_rules('objcopy-file')
        


   