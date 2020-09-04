

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <ps4sdk.h>
#include <orbislink.h>
#include <orbisGl.h>
#include <libkernel.h>  //sceKernelIccSetBuzzer


OrbisPadConfig *confPad;
bool flag=true;


int updateController(uint8_t *p)
//void updateController()
{
    unsigned int buttons=0;
    int ret = orbisPadUpdate();
    if(ret==0)
    {
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L1|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R1) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
// U
        if(orbisPadGetButtonPressed(ORBISPAD_UP))// || orbisPadGetButtonHold(ORBISPAD_UP))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Up pressed\n");
            p[82] = 1; ret = 82;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_UP))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Up released\n"); 
            p[82] = 0; ret = 0;
        }
// D
        if(orbisPadGetButtonPressed(ORBISPAD_DOWN))// || orbisPadGetButtonHold(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Down pressed\n");
            p[81] = 1;
            ret = 81;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Down released\n");
            p[81] = 0; ret = 0;
        }
// R
        if(orbisPadGetButtonPressed(ORBISPAD_RIGHT))// || orbisPadGetButtonHold(ORBISPAD_RIGHT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Right pressed\n");
            //p[80] = 0;
            p[79] = 1; 
            ret = 79;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_RIGHT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Right released\n");
            p[79] = 0; ret = 0;
        }
// L
        if(orbisPadGetButtonPressed(ORBISPAD_LEFT))// || orbisPadGetButtonHold(ORBISPAD_LEFT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Left pressed\n");
            p[80] = 1; ret = 80;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_LEFT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Left released\n");
            p[80] = 0; ret = 0;
        }
// TR
        if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Triangle pressed exit\n");

            flag=0;  // exit app
        }
// O
        if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Circle pressed\n");
            p[40] = 1;  ret = 40;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_CIRCLE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Circle Released\n");
            p[40] = 0;  ret = 0;
        }
// X
        if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Cross pressed\n");
            p[44] = 1;  ret = 44;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_CROSS))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Cross Released\n");
            p[44] = 0;  ret = 0;
        }
// SQ
        if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Square pressed\n");
        }
// L1
        if(orbisPadGetButtonPressed(ORBISPAD_L1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"L1 pressed\n");
            p[224] = 1; ret = 224;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_L1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"L1 Released\n");
            p[224] = 0;  ret = 0;
        }
// L2
        if(orbisPadGetButtonPressed(ORBISPAD_L2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"L2 pressed\n");
        }
// R1
        if(orbisPadGetButtonPressed(ORBISPAD_R1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"R1 pressed\n");
            p[226] = 1; ret = 226;
        } else
        if(orbisPadGetButtonReleased(ORBISPAD_R1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"R1 Released\n");
            p[226] = 0;  ret = 0;
        }
// R2
        if(orbisPadGetButtonPressed(ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"R2 pressed\n");
           // SDL_Log("%s test\n", __FUNCTION__);
            //SDL_Assert()
        }
    }
    return ret;
}

void finishApp()
{
    //orbisAudioFinish();
    //orbisKeyboardFinish();
    //orbisGlFinish();
    orbisPadFinish();
    orbisNfsFinish();
}

static bool initAppGl()
{
    int ret = orbisGlInit(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
    if(ret>0)
    {
        glViewport(0, 0, ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
        ret=glGetError();
        if(ret)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[%s] glViewport failed: 0x%08X\n",__FUNCTION__,ret);
            return false;
        }
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue RGBA
        ret=glGetError();
        if(ret)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[%s] glClearColor failed: 0x%08X\n",__FUNCTION__,ret);
            return false;
        }
        return true;
    }
    return false;
}

bool initApp()
{
    //orbisNfsInit(NFSEXPORT);
    //orbisFileInit();
//  int ret=initOrbisLinkApp();

    sceSystemServiceHideSplashScreen();


    confPad=orbisPadGetConf(); 

    if( ! initAppGl() ) return false;

    return true;
}


/// from cmd_build.c (this prototype is very important!)
char **build_cmd(char *cmd_line);


/// for timing, fps
#define WEN  (2048)
unsigned int frame   = 1,
             time_ms = 0;


int main(int argc, char *argv[])
{
    int ret = sceKernelIccSetBuzzer(1);    sleep(2);
    /*
        prepare pad and piglet modules in advance,
        read /data/orbislink/orbislink_config.ini for debugnet

        (no nfs, audio init yet...)
    */

    // debugNetInit("10.0.0.2", 18198, 3);
    ret = initOrbisLinkAppVanillaGl(); // will call loadModulesVanillaGl();
    if(ret)
    {
        sceKernelIccSetBuzzer(2); // notify error
        debugNetPrintf(DEBUGNET_ERROR, "%s: initOrbisLinkAppVanillaGl() ret: %d\n",__FUNCTION__, ret);
    }

    // init libraries
    flag=initApp();

    // init GLES2 stuff
//  ORBIS_RenderFillRects_init(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
    GLES2_Init_video(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);

    /// build a valid commanline and pass it to opentyrian main
//  char   cli[256] = "./egl_demo_opentyrian -s -j -t /usb0/tyrian21";
    char   cli[256] = "./egl_demo_opentyrian -s -j -t /app0/media";
    char **cmdline  = build_cmd(&cli[0]);
    _main(5, cmdline);

    /* NOT reached, opentyrian will keep control! */

    /// reset timer
    time_ms = get_time_ms();


    /// enter main render loop
    while(flag)
    {
        updateController(NULL);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ret = glGetError();
        if(ret) {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBIS_GL] glClear failed: 0x%08X\n", ret);
            //goto err;
        }

        /// draw
//      ORBIS_RenderFillRects_rndr();

        /// get timing, fps
        if( ! (frame %WEN) )
        {
            unsigned int now = get_time_ms();
            debugNetPrintf(DEBUGNET_INFO,"frame: %d, took: %ums, fps: %.3f\n", frame, now - time_ms,
                                                     ((double)WEN / (double)(now - time_ms) * 1000.f));
            time_ms = now;
        }
        frame++;

        orbisGlSwapBuffers();  /// flip frame


        sceKernelUsleep(1000);
    }
    
    // destructors
//  ORBIS_RenderFillRects_fini();
    finishOrbisLinkApp();

    exit(EXIT_SUCCESS);
}
