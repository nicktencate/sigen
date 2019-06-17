// Copyright 1999-2019 Ed Porras
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// pmt.h: class definition for the PMT
// -----------------------------------

#pragma once

#include <memory>
#include <list>
#include "table.h"

namespace sigen {

   class Descriptor;

   //
   // the pmt class
   //
   class PMT : public PSITable
   {
   public:
      // Elementary stream types
      enum esTypes {
         ES_RESERVED                                          = 0x00,
         ES_ISO_IEC_11172_VIDEO                               = 0x01,
         ES_ISO_IEC_13818_2_VIDEO                             = 0x02,
         ES_ISO_IEC_11172_AUDIO                               = 0x03,
         ES_ISO_IEC_13818_3_AUDIO                             = 0x04,
         ES_ISO_IEC_13818_1_PRIVATE_SECTIONS                  = 0x05,
         ES_ISO_IEC_13818_1_PES_PRIVATE_DATA                  = 0x06,
         ES_ISO_IEC_13522_MHEG                                = 0x07,
         ES_ISO_IEC_13818_1_ANNEX_A_DSM_CC                    = 0x08,
         ES_ITU_T_REC_H_222_1                                 = 0x09,
         ES_ISO_IEC_13818_6_TYPE_A                            = 0x0A,
         ES_ISO_IEC_13818_6_TYPE_B                            = 0x0B,
         ES_ISO_IEC_13818_6_TYPE_C                            = 0x0C,
         ES_ISO_IEC_13818_6_TYPE_D                            = 0x0D,
         ES_ISO_IEC_13818_1_AUXILIARY                         = 0x0E,
         ES_ISO_IEC_13818_7_AUDIO_ADTS_TS_SYNTAX              = 0x0F,
         ES_ISO_IEC_14496_2_VISUAL                            = 0x10,
         ES_ISO_IEC_14496_3_AUDIO_LATM                        = 0x11,
         ES_ISO_IEC_14496_1_STREAM_IN_PES_PACKETS             = 0x12,
         ES_ISO_IEC_14496_1_STREAM_IN_ISO_IEC_144496_SECTIONS = 0x13,
         ES_ISO_IEC_13818_6_SYNCHRONIZED_DOWNLOAD_PROTOCOL    = 0x14,
      };

      // constructor
      PMT(ui16 prog_num, ui16 pcrpid, ui8 ver, bool cni = true) :
         PSITable(TID, prog_num, 9, MAX_SEC_LEN, ver, cni, D_BIT),
         program_info_length(0),
         pcr_pid(pcrpid)
      { }

      bool addProgramDesc(Descriptor &);                   // add a program descriptor

      bool addElemStream(ui8 type, ui16 elem_pid);         // add an elementary stream
      bool addElemStreamDesc(ui16 elem_pid, Descriptor &); // add an elem stream desc to the stream with the given pid
      bool addElemStreamDesc(Descriptor &);                // adds it to the last stream added

#ifdef ENABLE_DUMP
      virtual void dump(std::ostream &) const;
#endif

   private:
      enum { D_BIT = 0,
             TID = 0x02,
             MAX_SEC_LEN = 1024 };

      // the stream holder struct - private to the pmt
      struct ElementaryStream {
         enum { BASE_LEN = 5 };

         ui16 es_info_length;
         ui16 elementary_pid : 13;
         ui8 type;
         DescList descriptors;

         // constructor
         ElementaryStream(ui16 epid, ui8 t) :
            es_info_length(0), elementary_pid(epid), type(t)
         { }

         bool writeSection(Section& s, ui16 max_data_len, ui16& sec_bytes) const;

      private:
         enum State_t { INIT, WRITE_HEAD, GET_DESC, WRITE_DESC };
         mutable struct Context {
            Context() : op_state(INIT), tsd(nullptr) {}

            State_t op_state;
            const Descriptor *tsd;
            std::list<std::unique_ptr<Descriptor> >::const_iterator tsd_iter;
         } run;
      };

      // instance variables
      ui16 program_info_length;
      ui16 pcr_pid : 13;
      DescList prog_desc;
      std::list<ElementaryStream> es_list; // the list of streams

      enum State_t { INIT, WRITE_HEAD, GET_PROG_DESC, WRITE_PROG_DESC,
                     GET_XPORT_STREAM, WRITE_XPORT_STREAM };
      mutable struct Context {
         Context() : d_done(false), op_state(INIT), pd(nullptr), es(nullptr) {}

         bool d_done;
         State_t op_state;
         const Descriptor *pd;
         const ElementaryStream *es;
         std::list<std::unique_ptr<Descriptor> >::const_iterator pd_iter;
         std::list<ElementaryStream>::const_iterator es_iter;
      } run;

   protected:
      bool addElemStreamDesc(ElementaryStream&, Descriptor &);
      virtual bool writeSection(Section&, ui8, ui16 &) const;
   };
} // sigen namespace

