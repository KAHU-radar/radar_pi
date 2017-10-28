
#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

// Note that the order of the ports is different on B and B. I guess someone
// in Navico just didn't realize this. Or it is just a bit of obfuscation.
static const NetworkAddress data4G_B = {{IPV4_ADDR(236, 6, 7, 13)}, IPV4_PORT(6657)};
static const NetworkAddress send4G_B = {{IPV4_ADDR(236, 6, 7, 14)}, IPV4_PORT(6658)};
static const NetworkAddress report4G_B = {{IPV4_ADDR(236, 6, 7, 15)}, IPV4_PORT(6659)};

PLUGIN_END_NAMESPACE

#endif

// 4G has 2048 spokes of exactly 512 bytes each
#if SPOKES_MBX < 2048
#undef SPOKES_MBX
#define SPOKES_MBX 2048
#endif
#if SPOKE_LEN_MBX < 512
#undef SPOKE_LEN_MBX
#define SPOKE_LEN_MBX 512
#endif

DEFINE_RADAR(RT_4GB,                                      /* Type */
             wxT("Navico 4G B"),                          /* Name */
             2048,                                        /* Spokes */
             512,                                         /* Spoke length (max) */
             NavicoControlsDialog(RT_4GB),                /* ControlsDialog class constructor */
             NavicoReceive(pi, ri, report4G_B, data4G_B), /* Receive class constructor */
             NavicoControl(send4G_B)                      /* Send/Control class constructor */
)