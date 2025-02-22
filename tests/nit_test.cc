#include "../src/sigen.h"
#include "dvb_builder.h"

using namespace sigen;

namespace tests
{
   int nit(TStream& t)
   {
      // NIT test
      NITActual nit(0x100, 0x01);

      nit.setMaxSectionLen( 300 ); // to test sectionizing

      // ----------------------------------------
      // add network descriptors

      // NetworkNameDesc
      NetworkNameDesc *nnd1 = new NetworkNameDesc("my network"); // allocated with operator new
      nit.addNetworkDesc( *nnd1 ); // always pass by ref
      // don't modify the descriptor once added to the table

      NetworkNameDesc *nnd2 = new NetworkNameDesc(std::string(259, 'c'));

      // TEST: data was 259 - descriptor should truncate to 253
      if (nnd2->length() != 257)
         return 1;

      nit.addNetworkDesc( *nnd2 );

      // StuffingDesc
      StuffingDesc *stuff1 = new StuffingDesc( 'z', 13 );

      nit.addNetworkDesc( *stuff1 );

      // second constructor
      std::string data("zzzzzzzzzzzzzzzzzzzzzzz");
      StuffingDesc *stuff2 = new StuffingDesc(data);

      if (stuff2->length() != (data.length() + 2))
         return 1;

      nit.addNetworkDesc(*stuff2);

      // MultilingualNetworkNameDesc
      MultilingualNetworkNameDesc *mlnnd = new MultilingualNetworkNameDesc;
      mlnnd->addText("fre", "France");
      mlnnd->addText("spa", "Francia");
      mlnnd->addText("eng", "France");
      mlnnd->addText("deu", "Frankreich");

      nit.addNetworkDesc( *mlnnd );

      // ----------------------------------------
      // add some transport streams
      nit.addXportStream(0x10, 0x20); // tsid, onid
      nit.addXportStream(0x11, 0x21);
      nit.addXportStream(0x20, 0x30);
      nit.addXportStream(0x21, 0x31);

      // and descriptors to the last TS added (0x21, 0x31)

      // SatelliteDeliverySystemDesc for DVB-S systems
      auto sdsd = new SatelliteDeliverySystemDesc(0x44444444, 0x3333, 0x1111111, false,
                                                  Dvb::Sat::LINEAR_VER_POL, Dvb::Sat::MOD_8PSK,
                                                  Dvb::CR_5_6_FECI);
      nit.addXportStreamDesc( *sdsd );

      // SatelliteDeliverySystemDesc for DVB-S2 systems.
      auto sdsd2 = new SatelliteDeliverySystemDesc(0x44444444, 0x3333, 0x1111111, true,
                                                   Dvb::Sat::CIRCULAR_RIGHT_POL, Dvb::Sat::MOD_QPSK,
                                                   Dvb::CR_9_10_FECI, Dvb::Sat::ROF_020);
      nit.addXportStreamDesc( *sdsd2 );

      // StreamIdentifierDesc
      StreamIdentifierDesc *sid = new StreamIdentifierDesc(0x88);
      nit.addXportStreamDesc( *sid );

      // TimeShiftedServiceDesc
      TimeShiftedServiceDesc *tssd = new TimeShiftedServiceDesc( 0x4000 );
      nit.addNetworkDesc( *tssd );

      // AnnouncementSupportDesc
      auto asd = new AnnouncementSupportDesc(AnnouncementSupportDesc::EMERGENCY_ALARM_AS |
                                             AnnouncementSupportDesc::WEATHER_FLASH_AS);

      // test adding a ref_type 0x0
      asd->addAnnouncement(AnnouncementSupportDesc::SERVICE_AUDIO_STREAM_RT);

      // test adding a ref_type with service info
      asd->addAnnouncement(AnnouncementSupportDesc::WEATHER_FLASH_AT,
                           AnnouncementSupportDesc::DIFFERENT_SERVICE_RT,
                           0x1100, 0x1300, 0x402, 0x3);

      nit.addXportStreamDesc( *asd );

      // CellFrequencyLinkDesc
      CellFrequencyLinkDesc* cfld = new CellFrequencyLinkDesc;
      cfld->addCell(10, 10000);
      cfld->addCell(10, 11000);
      cfld->addSubCell(10, 11, 1000);
      cfld->addSubCell(10, 12, 3300);
      cfld->addSubCell(12, 3000); // adds to the last cell added
      cfld->addCell(11, 40000);
      cfld->addSubCell(11, 21, 2000);
      nit.addXportStreamDesc(*cfld);

      // CellListDesc
      CellListDesc* cld = new CellListDesc;
      cld->addCell(1, 3000, 2000, 555, 65);
      cld->addSubCell(1, 20, 3000, 2000, 555, 65);
      cld->addSubCell(21, 3001, 2001, 556, 66);
      cld->addCell(2, 4000, 3000, 5555, 655);
      cld->addSubCell(22, 3002, 2002, 557, 668);
      nit.addXportStreamDesc(*cld);

      // FrequencyListDesc
      FrequencyListDesc *fld = new FrequencyListDesc( 0x2 );
      fld->addFrequency( 0x1000 );
      fld->addFrequency( 0x2000 );
      fld->addFrequency( 0x3000 );
      fld->addFrequency( 0x4000 );
      fld->addFrequency( 0x5000 );
      fld->addFrequency( 0x6000 );

      nit.addXportStreamDesc( *fld );

      // add descriptors to a TS by id
      ui16 tsid = 0x20;

      auto ccd = new CableDeliverySystemDesc(1000, 2000, 0x01, 0x08, 0x02);
      nit.addXportStreamDesc( tsid, *ccd );

      // TerrestrialDeliverySystemDesc - legacy constructor
      auto tdsd = new TerrestrialDeliverySystemDesc(0x88888888,
                                                    Dvb::Terr::BW_5_MHZ,
                                                    Dvb::Terr::CONS_QAM_64,
                                                    Dvb::Terr::HI_1_NATIVE,
                                                    Dvb::Terr::CR_2_3, Dvb::Terr::CR_5_6,
                                                    Dvb::Terr::GI_1_4,
                                                    Dvb::Terr::TM_4K,
                                                    false);
      nit.addXportStreamDesc( tsid, *tdsd );

      // TerrestrialDeliverySystemDesc - new constructor
      auto tdsd2 = new TerrestrialDeliverySystemDesc(0x55555555,
                                                     Dvb::Terr::BW_6_MHZ,
                                                     Dvb::Terr::CONS_QAM_16,
                                                     Dvb::Terr::HI_4_IN_DEPTH,
                                                     Dvb::Terr::CR_2_3, Dvb::Terr::CR_7_8,
                                                     Dvb::Terr::GI_1_16,
                                                     Dvb::Terr::TM_8K,
                                                     true,
                                                     Dvb::Terr::PRI_HIGH,
                                                     false,
                                                     false);
      nit.addXportStreamDesc( tsid, *tdsd2 );

      DUMP(nit);
      nit.buildSections(t);

      // dump built sections
      DUMP(t);

      return tests::cmp_bin(t, "reference/nit.ts");
   }
}
