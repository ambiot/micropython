#include "objsdfs.h"

#define TEST_SIZE   (512)

char WRBuf[TEST_SIZE];
char RDBuf[TEST_SIZE];
int sdioInitErr = FR_OK;

static void *m_fs = NULL;
static void *m_file = NULL;
static char logical_drv[4] = "0:/";
char mp_cwd[TEST_SIZE];
bool dirChanged = false;

#if 0
logical_drv[0] = '0';
logical_drv[1] = ':';
logical_drv[2] = '/';
logical_drv[3] = '\0';
#endif

static int drv_num = -1;

bool mounted_flash = false;

STATIC sdfs_obj_t sdfs_obj[1] = {{.base.type = &sdfs_type, .unit = 0 }};


///////////////////////////////
//                           //
//    Internal Functions     //
//                           //
///////////////////////////////


bool init_sd_fs(void){
	FRESULT ret = FR_OK;

    do {
        m_fs = (FATFS *) malloc (sizeof(FATFS));
        if (m_fs == NULL) {
            ret = FR_INT_ERR;
            break;
        }

        if(sdioInitErr == FR_DISK_ERR)
            break;
        
        // Uncomment the line below to select between flash and SD card
        // Note that disk specific macro has to be set&clear in platform_opts.h to enable fs
        // also SD card uses 512 for _MAX_SS, and flash 4096 in ffconf.h

        drv_num = FATFS_RegisterDiskDriver(&SD_disk_Driver);  // mount on SD card
        //drv_num = FATFS_RegisterDiskDriver(&FLASH_disk_Driver);   //mount on flash
        
        if (drv_num < 0) {
            printf("Rigester disk driver to FATFS fail.\n");
            ret = FR_DISK_ERR;
            break;
        }

        logical_drv[0] += drv_num;

        ret = f_mount((FATFS *)m_fs, logical_drv, 1);
        if (ret != FR_OK){
            printf("FATFS mount logical drive fail:%d\n", ret);
            break;
        }

    } while (0);

    if (ret != FR_OK) {
        drv_num = -1;
    }

    memset(mp_cwd, 0, TEST_SIZE);
    sprintf(mp_cwd, "%s", logical_drv);

    return (-(int)ret);
}


char* getRootPath() {
    if (drv_num < 0) {
        return NULL;
    } else {
        return logical_drv;
    }
}



void open(char* fileName) {
    FRESULT ret = FR_OK;
    char absolute_path[128];

    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        m_file = (FIL *)malloc (sizeof(FIL));
        if (m_file == NULL) {
            ret = FR_INT_ERR;
            break;
        }

        sprintf(absolute_path, "%s/%s", mp_cwd, fileName);

        ret = f_open((FIL *)m_file, absolute_path, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

        if (ret != FR_OK) {
            printf("open file (%s) fail. (ret=%d)\n", absolute_path, ret);
            break;
        }
    } while (0);

    if (ret != FR_OK) {
        if (m_file != NULL) {
            free(m_file);
            m_file = NULL;
        }
    }

    //return m_file; // TBD, here should return an object of sdfs, not pointer
}


#if 0
char* getCWD() {

    char absolute_path[128];

    if (drv_num < 0) {
        return NULL;
    } 
    
    f_getcwd(absolute_path, 128);

    return absolute_path;
}
#endif


///////////////////////////////
//                           //
//    External Functions     //
//                           //
///////////////////////////////


STATIC mp_obj_t sdfs_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    const mp_arg_t sdfs_init_args[] = {
        { MP_QSTR_unit, MP_ARG_INT, {.u_int = 0} },   // unit 0 is SD card file system 
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(sdfs_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), sdfs_init_args, args);

    int unit = args[0].u_int;

    sdfs_obj_t *self = &sdfs_obj[unit];

    // Initialise the local flash filesystem.
    mounted_flash = init_sd_fs();  // mount on sd card

    if (!mounted_flash) {
        printf("[MP]: SD file system mount success\n");
    } else {
        printf("[MP]: Failed to mount SD file system\n");
    }

    return self;
}



STATIC mp_obj_t listdir(mp_obj_t self_in) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    FILINFO fno;
    DIR dir;
    unsigned int bufsize = 512;
    char result_buf[bufsize];

    char *fn;
    unsigned int fnlen;
    int bufidx = 0;

    char* fn2print;


#if _USE_LFN
    char lfn[(_MAX_LFN + 1)];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif

    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        // open current directory
        if(dirChanged) {
            ret = f_opendir(&dir, mp_cwd);
        } else {
            ret = f_opendir(&dir, getRootPath());
        }


        if (ret != FR_OK) {
            break;
        }

        memset(result_buf, 0, bufsize);

        while (1) {
            ret = f_readdir(&dir, &fno);
            if ((ret != FR_OK) || (fno.fname[0] == 0)) {
                break;
            }

#if _USE_LFN
            if (*fno.lfname)
            {
                fn = fno.lfname;
                fnlen = fno.lfsize;
            }
            else
#endif
            {
                fn = fno.fname;
                fnlen = strlen(fn);
            }

            if ((bufidx + fnlen + 1) < bufsize) {
                bufidx += sprintf((result_buf + bufidx), "%s", fn);
                bufidx++;
            }
        }
    } while (0);

    /* the filenames are separated with '\0', so we scan one by one */
    fn2print = result_buf + strlen(result_buf) + 1;
    while(strlen(fn2print) > 0) {
        printf("%s\r\n", fn2print);
        fn2print += strlen(fn2print) + 1;
    }

    //return mp_obj_new_str(result_buf, bufidx);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(listdir_obj, listdir);



STATIC mp_obj_t mkdir(mp_obj_t self_in, mp_obj_t dirName) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    char absolute_path[128];
    
    char* dir_path = mp_obj_str_get_str(dirName);
    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        sprintf(absolute_path, "%s%s", getRootPath(), dir_path);

        ret = f_mkdir(absolute_path);
        if (ret != FR_OK) {
            break;
        }
    } while (0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mkdir_obj, mkdir);



// Note: Set _FS_RPATH to 2 in ffconf.h in order to enable chdir and getcwd
STATIC mp_obj_t chdir(mp_obj_t self_in, mp_obj_t dirName) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    char absolute_path[128];
    char root[] = "/";
    
    char* dir_path = mp_obj_str_get_str(dirName);
    //printf("dir path keyed in is :%s\n", dir_path);
    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        if (!strcmp(dir_path, root)) { // cd back to root dir
            dirChanged = false;
            sprintf(absolute_path, "%s", getRootPath());
            ret = f_chdir(absolute_path);
        } else {
            if (dirChanged) {  // cd to 2 and above layer of dir
                sprintf(absolute_path, "%s/%s", mp_cwd, dir_path);
            } else {  // cd to 2nd layer dir
                sprintf(absolute_path, "%s%s", getRootPath(), dir_path);
            }

            ret = f_chdir(absolute_path);
            if (ret != FR_OK) {
                break;
            }
        }
    } while (0);

    // update flag
    dirChanged = true;

    //update mp_cwd
    memset(mp_cwd, 0, TEST_SIZE);
    sprintf(mp_cwd, "%s", absolute_path);
    //printf("mp_cwd is %s\n", mp_cwd);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(chdir_obj, chdir);



STATIC mp_obj_t rm(mp_obj_t self_in, mp_obj_t fileName) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    char absolute_path[128];
    
    char* fn = mp_obj_str_get_str(fileName);
    
    sprintf(absolute_path, "%s%s", getRootPath(), fn);

    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        ret = f_unlink(absolute_path);
        if (ret != FR_OK) {
            break;
        }
    } while (0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rm_obj, rm);



STATIC mp_obj_t pwd(mp_obj_t self_in) {
    sdfs_obj_t *self = self_in;
    printf("%s\n", mp_cwd);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(pwd_obj, pwd);



STATIC mp_obj_t create(mp_obj_t self_in, mp_obj_t fileName) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    char absolute_path[128];

    char* fn = mp_obj_str_get_str(fileName);
    do {
        if (drv_num < 0) {
            ret = FR_DISK_ERR;
            break;
        }

        m_file = (FIL *)malloc (sizeof(FIL));
        if (m_file == NULL) {
            ret = FR_INT_ERR;
            break;
        }

        sprintf(absolute_path, "%s/%s", mp_cwd, fn);

        ret = f_open((FIL *)m_file, absolute_path, FA_CREATE_ALWAYS);

        if (ret != FR_OK) {
            printf("open file (%s) fail. (ret=%d)\n", absolute_path, ret);
            break;
        }
    } while (0);


    //close the file
    f_close((FIL *)m_file);
    free(m_file);
    m_file = NULL;

    if (ret != FR_OK) {
        mp_raise_ValueError("Failed to create file");
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(create_obj, create);



STATIC mp_obj_t read(mp_obj_t self_in, mp_obj_t fileName) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    char buf[512];
    unsigned int readsize = 0;
    unsigned char nbyte = 127;

    char* fn = mp_obj_str_get_str(fileName);
    open(fn);

    ret = f_read((FIL *)m_file, buf, nbyte, &readsize);

    if (ret != FR_OK) {
        printf("File function error. \r\n");
    } else {
        printf("%s \r\n", buf);
    }

    //close the file
    f_close((FIL *)m_file);
    free(m_file);
    m_file = NULL;

    //return mp_obj_new_int(readsize);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(read_obj, read);



STATIC mp_obj_t write(mp_obj_t self_in, mp_obj_t fileName, mp_obj_t buf_in) {
    sdfs_obj_t *self = self_in;
    FRESULT ret = FR_OK;
    unsigned int writesize = 0;
    char buf[512];

    char* fn = mp_obj_str_get_str(fileName);
    open(fn);

#if 0
    //Prepare the buffer to write from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

    ret = f_write((FIL *)m_file, (const void *)bufinfo.buf, bufinfo.len, &writesize);
#endif

    memset(&buf[0], 0x00, 512);

    char* buf_temp = mp_obj_str_get_str(buf_in);

    strcpy(&buf[0], buf_temp);

    //printf("Recv: %s, commencing f write\n", buf_temp);

    ret = f_write((FIL *)m_file, (const void *)buf, sizeof(buf), &writesize);

    //printf("test f write finish\n");
    if (ret != FR_OK) {
        printf("File function error. \r\n");
    }

    //close the file
    f_close((FIL *)m_file);
    free(m_file);
    m_file = NULL;

    //return writesize;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(write_obj, write);



/////////////////////////////////////////////////
//                                             //
//            MicroPython API                  //
//                                             //
/////////////////////////////////////////////////


STATIC const mp_map_elem_t sdfs_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_listdir),       MP_OBJ_FROM_PTR(&listdir_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_mkdir),         MP_OBJ_FROM_PTR(&mkdir_obj)   },
    { MP_OBJ_NEW_QSTR(MP_QSTR_chdir),         MP_OBJ_FROM_PTR(&chdir_obj)   },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pwd),           MP_OBJ_FROM_PTR(&pwd_obj)     },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rm),            MP_OBJ_FROM_PTR(&rm_obj)      },
    { MP_OBJ_NEW_QSTR(MP_QSTR_create),        MP_OBJ_FROM_PTR(&create_obj)  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),          MP_OBJ_FROM_PTR(&read_obj)    },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),         MP_OBJ_FROM_PTR(&write_obj)   },
};
STATIC MP_DEFINE_CONST_DICT(sdfs_locals_dict, sdfs_locals_dict_table);


const mp_obj_type_t sdfs_type = {
    { &mp_type_type },
    .name        = MP_QSTR_SDFS,
    .make_new    = sdfs_make_new,
    .locals_dict = (mp_obj_t)&sdfs_locals_dict,
};
