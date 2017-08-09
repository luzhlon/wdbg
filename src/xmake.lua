
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
    end

    on_package(function(target)
        os.cp('$(projectdir)/deps/srpc/src/*.h*', 'dist/inc')
        os.cp('$(projectdir)/deps/xval/src/*.h', 'dist/inc')
        os.cp('$(projectdir)/deps/dbgeng/inc/*.h', 'dist/inc')
        os.cp('$(projectdir)/src/*.h', 'dist/inc')
        os.cp('$(projectdir)/deps/dbgeng/lib/$(arch)', 'dist/lib')
        os.cp('$(projectdir)/deps/dbgeng/$(arch)/dbgeng.dll', 'dist/bin/$(arch)')
        os.cp('$(buildir)/$(mode)/$(arch)/srpc.dll', 'dist/bin/$(arch)')
        os.cp('$(buildir)/$(mode)/$(arch)/wdbg.exe', 'dist/bin/$(arch)')
        os.cp('$(buildir)/srpc.lib', 'dist/lib/$(arch)')
        os.cp('$(buildir)/wdbg.lib', 'dist/lib/$(arch)')
        os.cp('$(projectdir)/pywdbg', 'dist')
        os.cp('$(projectdir)/examples', 'dist')
    end)

    add_deps 'srpc'
