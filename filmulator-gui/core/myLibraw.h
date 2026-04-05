#ifndef MYLIBRAW_H
#define MYLIBRAW_H

#include <iostream>
#include <libraw/libraw.h>

class MyLibRaw : public LibRaw
{
public:
  void my_phase_one_free_tempbuffer() { phase_one_free_tempbuffer(); }
  int phaseone_fix(bool &needs_phase_one_free)
  {
    std::cout << "MyLibRaw is phaseone compressed: " << is_phaseone_compressed() << std::endl;
    std::cout << "MyLibRaw split_col: " << imgdata.color.phase_one_data.split_col << std::endl;
    std::cout << "MyLibRaw split_row: " << imgdata.color.phase_one_data.split_row << std::endl;
    if (is_phaseone_compressed() && imgdata.rawdata.raw_alloc) {
      phase_one_allocate_tempbuffer();
      needs_phase_one_free = true;
      int rc = phase_one_subtract_black((ushort *)imgdata.rawdata.raw_alloc, imgdata.rawdata.raw_image);
      if (rc == 0 && imgdata.params.use_p1_correction) {
        std::cout << "using p1 correction" << std::endl;
        rc = phase_one_correct();
        std::cout << "after using p1 correction" << std::endl;
        return rc;
      }
      return rc;
    }
    return 0;
  }
};

#endif// MYLIBRAW_H
