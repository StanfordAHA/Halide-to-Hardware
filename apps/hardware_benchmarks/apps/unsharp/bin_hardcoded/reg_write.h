

static void bitstream_glb_config()
{
glb_reg_write(0xec, 0x1);
glb_reg_write(0xf0, 0x0);
glb_reg_write(0xf4, 0x651);
glb_reg_write(0xf8, 0x14);
glb_reg_write(0x1f8, 0x28);
glb_reg_write(0x2f8, 0x28);
glb_reg_write(0x3f8, 0x28);
glb_reg_write(0x4f8, 0x28);
glb_reg_write(0x5f8, 0x28);
glb_reg_write(0x6f8, 0x28);
glb_reg_write(0x7f8, 0x28);
glb_reg_write(0x8f8, 0x28);
glb_reg_write(0x9f8, 0x28);
glb_reg_write(0xaf8, 0x28);
glb_reg_write(0xbf8, 0x28);
glb_reg_write(0xcf8, 0x28);
glb_reg_write(0xdf8, 0x28);
}


static void app_glb_config()
{
glb_reg_write(0x18, 0x0);
glb_reg_write(0x7c, 0x51);
glb_reg_write(0x80, 0x1);
glb_reg_write(0x84, 0x0);
glb_reg_write(0x88, 0x0);
glb_reg_write(0x8c, 0x43fe);
glb_reg_write(0x94, 0x1);
glb_reg_write(0x90, 0x2);
glb_reg_write(0x118, 0x0);
glb_reg_write(0x17c, 0x51);
glb_reg_write(0x180, 0x1);
glb_reg_write(0x184, 0x20000);
glb_reg_write(0x188, 0x0);
glb_reg_write(0x18c, 0x43fe);
glb_reg_write(0x194, 0x1);
glb_reg_write(0x190, 0x2);
glb_reg_write(0x218, 0x0);
glb_reg_write(0x27c, 0x51);
glb_reg_write(0x280, 0x1);
glb_reg_write(0x284, 0x40000);
glb_reg_write(0x288, 0x0);
glb_reg_write(0x28c, 0x43fe);
glb_reg_write(0x294, 0x1);
glb_reg_write(0x290, 0x2);
glb_reg_write(0x18, 0x0);
glb_reg_write(0x8, 0x2d);
glb_reg_write(0xc, 0x1);
glb_reg_write(0x10, 0x1);
glb_reg_write(0x1c, 0x2);
glb_reg_write(0x20, 0x10000);
glb_reg_write(0x24, 0x19e);
glb_reg_write(0x28, 0x3c);
glb_reg_write(0x30, 0x1);
glb_reg_write(0x2c, 0x2);
glb_reg_write(0x34, 0xf8);
glb_reg_write(0x3c, 0x7);
glb_reg_write(0x38, 0x2);
glb_reg_write(0x118, 0x0);
glb_reg_write(0x108, 0x2d);
glb_reg_write(0x10c, 0x1);
glb_reg_write(0x110, 0x1);
glb_reg_write(0x11c, 0x2);
glb_reg_write(0x120, 0x30000);
glb_reg_write(0x124, 0x19e);
glb_reg_write(0x128, 0x3c);
glb_reg_write(0x130, 0x1);
glb_reg_write(0x12c, 0x2);
glb_reg_write(0x134, 0xf8);
glb_reg_write(0x13c, 0x7);
glb_reg_write(0x138, 0x2);
glb_reg_write(0x218, 0x0);
glb_reg_write(0x208, 0x2d);
glb_reg_write(0x20c, 0x1);
glb_reg_write(0x210, 0x1);
glb_reg_write(0x21c, 0x2);
glb_reg_write(0x220, 0x50000);
glb_reg_write(0x224, 0x19e);
glb_reg_write(0x228, 0x3c);
glb_reg_write(0x230, 0x1);
glb_reg_write(0x22c, 0x2);
glb_reg_write(0x234, 0xf8);
glb_reg_write(0x23c, 0x7);
glb_reg_write(0x238, 0x2);
}
