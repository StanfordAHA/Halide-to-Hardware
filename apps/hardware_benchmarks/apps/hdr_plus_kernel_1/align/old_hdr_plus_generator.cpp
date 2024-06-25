#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "HalideBuffer.h"
#include "halide_image_io.h"

#include "align/align.h"
//#include "motion/motion.h"
//#include "merge/merge.h"
//#include "finish/finish.h"
#include "dirent.h"
//#include "tiny_dng_loader.h"

using namespace Halide::Tools;
using namespace Halide::Runtime;
using namespace align;

class HDR {
public:
    Buffer<uint8_t> imgs;

    HDR(Buffer<uint8_t> imgs) : imgs(imgs) {}

    Buffer<uint8_t> process() {
        Buffer<int16_t> align_out(imgs.width()/16, imgs.height()/16, 3, imgs.extent(3));
        Buffer<uint8_t> align_rgb(imgs.width()/16, imgs.height()/16, 3, imgs.extent(3));
        Buffer<uint8_t> merge_out(imgs.width(), imgs.height(), 3);
        Buffer<uint8_t> output(imgs.width(), imgs.height(), 3);
        
        //Alignment aligner;
        //aligner.input = imgs;
        //aligner.output= align_out;

        align(imgs, align_out);
        //motion_to_RGB(align_out, align_rgb);
        //merge(imgs, align_out, merge_out);
        //finish(merge_out, output);
        //save_imgs("output", align_rgb);
        save_imgs("output", align_out);
        
        return output;
    }

    static bool load_imgs(std::string dir_path, Buffer<uint8_t> &imgs) {
        std::vector<std::string> img_names;

        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir(dir_path.c_str())) != NULL) {
            std::string file_name;
            std::string ext;
            while ((ent = readdir(dir)) != NULL) {
                file_name = std::string(ent->d_name);
                ext = Internal::get_lowercase_extension(file_name);
                if (ext != "png")
                    continue;
                img_names.push_back(file_name);
            }
            std::sort(img_names.begin(), img_names.end());
            closedir(dir);
        } else {
          perror ("");
          return EXIT_FAILURE;
        }

        Buffer<uint8_t> img_tmp = load_image(dir_path + "/" + img_names[0]); 
        int num_imgs = img_names.size();
        int width, height, channel;
        int ref = num_imgs/2;
        for (int i = ref; i > 0 ; i--) {
            std::iter_swap(img_names.begin() + i, img_names.begin() + i - 1);
        }

        for (int i = 0; i < num_imgs; i++) {
            img_tmp = load_image(dir_path + "/" + img_names[i]);
            if (i == 0) {
                width = img_tmp.width();
                height = img_tmp.height();
                channel = img_tmp.channels();
                imgs = Buffer<uint8_t>(width, height, channel, num_imgs);
                printf("width: %d, height: %d, channel: %d, number of images: %d\n", 
                        width, height, channel, num_imgs);
            }
            printf("loaded %s image\n", img_names[i].c_str());
            imgs.sliced(3, i).copy_from(img_tmp);
        }

        return true;
    }
    static bool save_imgs(std::string dir_path, Buffer<uint8_t> &imgs) {
        int num_imgs = imgs.extent(3);
        int ref = num_imgs / 2;
        int target;
        Buffer<uint8_t> output;
            
        for (int i = 0; i < num_imgs; i++) {
            output = imgs.sliced(3, i);
            if (i == 0)
                target = ref;
            else if (i <= ref)
                target = i - 1;
            else
                target = i;
            save_image(output, dir_path + "/output" + std::to_string(target) + ".png");
            printf("saved output%d.png\n", target);
        }

        return true;
    }
};

int main(int argc, char **argv) {
    
    Buffer<uint8_t> imgs;
    if(!HDR::load_imgs(argv[1], imgs))
        return -1;

    HDR hdr = HDR(imgs);
    Buffer<uint8_t> output = hdr.process();
    save_image(output, "output/output.png");

    return 0;
}
