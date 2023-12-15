TEMPLATE = subdirs

unix {
    SUBDIRS += \
        dde_lib/utils/IPCmemLib/ipcmem_lib.pro
}

SUBDIRS += \
    dde_lib/DDE_lib.pro \
    dde_lib/utils/sql3_db \
#   unicon_testpanel \
    gabbi_convertor \
    unicon_core


# unix: SUBDIRS += \
#    dde_lib/utils/parser
