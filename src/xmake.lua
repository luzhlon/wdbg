
local function copy_dbgengfiles(os, dest)
    os.cp('$(projectdir)/deps/dbgeng/$(arch)/*.dll', dest)
    os.cp('$(projectdir)/deps/dbgeng/$(arch)/winext', dest)
    os.cp('$(projectdir)/deps/dbgeng/$(arch)/winxp', dest)
end

target("wdbg")
    set_kind 'binary'

    set_targetdir('$(buildir)/$(mode)/$(arch)')

    add_includedirs('../deps/xval/src',
                    '../deps/srpc/src',
                    '../deps/dbgeng/inc')

    add_linkdirs('$(buildir)/$(mode)/$(arch)',
                '$(buildir)',
                '../deps/dbgeng/lib/$(arch)')
    add_links('srpc', 'dbgeng')

    add_files("*.cpp")
    add_defines('_WDBG_EXPORT')

    if is_mode 'debug' then
        add_cxxflags '/MDd'
    else
        add_cxxflags '/MD'
    end

    on_package(function(target)
        -- copy file for plugin development
        os.cp('$(projectdir)/deps/srpc/src/*.h*', 'wdbg-dist/inc')
        os.cp('$(projectdir)/deps/xval/src/*.h', 'wdbg-dist/inc')
        os.cp('$(projectdir)/src/*.h', 'wdbg-dist/inc')
        os.cp('$(buildir)/srpc.lib', 'wdbg-dist/lib/$(arch)')
        os.cp('$(buildir)/wdbg.lib', 'wdbg-dist/lib/$(arch)')
        os.cp('$(projectdir)/deps/dbgeng/inc/*.h', 'wdbg-dist/inc')
        os.cp('$(projectdir)/deps/dbgeng/lib/$(arch)', 'wdbg-dist/lib')
        -- copy file about dbgeng
        copy_dbgengfiles(os, 'wdbg-dist/bin/$(arch)')
        -- copy the binary files
        os.cp('$(buildir)/$(mode)/$(arch)/srpc.dll', 'wdbg-dist/bin/$(arch)')
        os.cp('$(buildir)/$(mode)/$(arch)/wdbg.exe', 'wdbg-dist/bin/$(arch)')
        -- copy the script files
        os.cp('$(projectdir)/pywdbg', 'wdbg-dist')
        os.cp('$(projectdir)/examples', 'wdbg-dist')
    end)

    add_deps 'srpc'

task 'copyfiles'

    on_run(function ()
        import 'core.project.config'
        config.load()
        copy_dbgengfiles(os, '$(buildir)/$(mode)/$(arch)')
    end)

    set_menu {
        usage = "xmake wdbgcopy [options]",
        description = "Copy the dbgeng files to build directory",
        options = {}
    }
