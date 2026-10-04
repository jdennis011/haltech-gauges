<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.7.0">
  <drawing>
    <settings>
      <setting alwaysvectorfont="no" />
      <setting verticaltext="up" />
    </settings>
    <grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch" />
    <layers>
      <layer number="1" name="Top" color="4" fill="1" visible="no" active="no" />
      <layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no" />
      <layer number="17" name="Pads" color="2" fill="1" visible="no" active="no" />
      <layer number="18" name="Vias" color="2" fill="1" visible="no" active="no" />
      <layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no" />
      <layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no" />
      <layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no" />
      <layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no" />
      <layer number="25" name="tNames" color="7" fill="1" visible="no" active="no" />
      <layer number="26" name="bNames" color="7" fill="1" visible="no" active="no" />
      <layer number="27" name="tValues" color="7" fill="1" visible="no" active="no" />
      <layer number="28" name="bValues" color="7" fill="1" visible="no" active="no" />
      <layer number="29" name="tStop" color="7" fill="1" visible="no" active="no" />
      <layer number="30" name="bStop" color="7" fill="1" visible="no" active="no" />
      <layer number="31" name="tCream" color="7" fill="1" visible="no" active="no" />
      <layer number="32" name="bCream" color="7" fill="1" visible="no" active="no" />
      <layer number="39" name="tKeepout" color="4" fill="1" visible="no" active="no" />
      <layer number="40" name="bKeepout" color="1" fill="1" visible="no" active="no" />
      <layer number="41" name="tRestrict" color="4" fill="1" visible="no" active="no" />
      <layer number="42" name="bRestrict" color="1" fill="1" visible="no" active="no" />
      <layer number="43" name="vRestrict" color="2" fill="1" visible="no" active="no" />
      <layer number="44" name="Drills" color="7" fill="1" visible="no" active="no" />
      <layer number="45" name="Holes" color="7" fill="1" visible="no" active="no" />
      <layer number="46" name="Milling" color="3" fill="1" visible="no" active="no" />
      <layer number="47" name="Measures" color="7" fill="1" visible="no" active="no" />
      <layer number="48" name="Document" color="7" fill="1" visible="no" active="no" />
      <layer number="49" name="Reference" color="7" fill="1" visible="no" active="no" />
      <layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no" />
      <layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no" />
      <layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes" />
      <layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes" />
      <layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes" />
      <layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes" />
      <layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes" />
      <layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes" />
      <layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes" />
      <layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes" />
    </layers>
    <schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
      <libraries>
        <library name="haltech-gauges">
          <packages>
            <package name="0805">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-0.95" y="0" dx="1" dy="1.3" layer="1" />
              <smd name="2" x="0.95" y="0" dx="1" dy="1.3" layer="1" />
              <text x="-0.95" y="1.05" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="1206">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-1.5" y="0" dx="1.2" dy="1.8" layer="1" />
              <smd name="2" x="1.5" y="0" dx="1.2" dy="1.8" layer="1" />
              <text x="-1.5" y="1.3" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="1812">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-2.05" y="0" dx="1.6" dy="3.5" layer="1" />
              <smd name="2" x="2.05" y="0" dx="1.6" dy="3.5" layer="1" />
              <text x="-2.05" y="2.15" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="SMA">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout. Pad 1 is the cathode.</description>
              <smd name="1" x="-2" y="0" dx="2.2" dy="1.7" layer="1" />
              <smd name="2" x="2" y="0" dx="2.2" dy="1.7" layer="1" />
              <text x="-2" y="1.25" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="SMB">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-2.15" y="0" dx="2.3" dy="2.3" layer="1" />
              <smd name="2" x="2.15" y="0" dx="2.3" dy="2.3" layer="1" />
              <text x="-2.15" y="1.55" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="CAP-SMD-8X10">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout. Pad 1 is positive.</description>
              <smd name="1" x="-3.2" y="0" dx="3.4" dy="1.6" layer="1" />
              <smd name="2" x="3.2" y="0" dx="3.4" dy="1.6" layer="1" />
              <text x="-3.2" y="1.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="SOT23">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-0.95" y="-1.1" dx="0.9" dy="1" layer="1" />
              <smd name="2" x="0.95" y="-1.1" dx="0.9" dy="1" layer="1" />
              <smd name="3" x="0" y="1.1" dx="0.9" dy="1" layer="1" />
              <text x="-1.4" y="2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="SOIC8">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout.</description>
              <smd name="1" x="-2.7" y="1.905" dx="1.55" dy="0.6" layer="1" />
              <smd name="8" x="2.7" y="1.905" dx="1.55" dy="0.6" layer="1" />
              <smd name="2" x="-2.7" y="0.635" dx="1.55" dy="0.6" layer="1" />
              <smd name="7" x="2.7" y="0.635" dx="1.55" dy="0.6" layer="1" />
              <smd name="3" x="-2.7" y="-0.635" dx="1.55" dy="0.6" layer="1" />
              <smd name="6" x="2.7" y="-0.635" dx="1.55" dy="0.6" layer="1" />
              <smd name="4" x="-2.7" y="-1.905" dx="1.55" dy="0.6" layer="1" />
              <smd name="5" x="2.7" y="-1.905" dx="1.55" dy="0.6" layer="1" />
              <wire x1="-1.95" y1="2.45" x2="1.95" y2="2.45" width="0.127" layer="21" />
              <wire x1="1.95" y1="2.45" x2="1.95" y2="-2.45" width="0.127" layer="21" />
              <wire x1="1.95" y1="-2.45" x2="-1.95" y2="-2.45" width="0.127" layer="21" />
              <wire x1="-1.95" y1="-2.45" x2="-1.95" y2="2.45" width="0.127" layer="21" />
              <text x="-1.95" y="2.9" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="SOIC16W">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout. Wide body, 7.5 mm.</description>
              <smd name="1" x="-4.7" y="4.445" dx="2" dy="0.6" layer="1" />
              <smd name="16" x="4.7" y="4.445" dx="2" dy="0.6" layer="1" />
              <smd name="2" x="-4.7" y="3.175" dx="2" dy="0.6" layer="1" />
              <smd name="15" x="4.7" y="3.175" dx="2" dy="0.6" layer="1" />
              <smd name="3" x="-4.7" y="1.905" dx="2" dy="0.6" layer="1" />
              <smd name="14" x="4.7" y="1.905" dx="2" dy="0.6" layer="1" />
              <smd name="4" x="-4.7" y="0.635" dx="2" dy="0.6" layer="1" />
              <smd name="13" x="4.7" y="0.635" dx="2" dy="0.6" layer="1" />
              <smd name="5" x="-4.7" y="-0.635" dx="2" dy="0.6" layer="1" />
              <smd name="12" x="4.7" y="-0.635" dx="2" dy="0.6" layer="1" />
              <smd name="6" x="-4.7" y="-1.905" dx="2" dy="0.6" layer="1" />
              <smd name="11" x="4.7" y="-1.905" dx="2" dy="0.6" layer="1" />
              <smd name="7" x="-4.7" y="-3.175" dx="2" dy="0.6" layer="1" />
              <smd name="10" x="4.7" y="-3.175" dx="2" dy="0.6" layer="1" />
              <smd name="8" x="-4.7" y="-4.445" dx="2" dy="0.6" layer="1" />
              <smd name="9" x="4.7" y="-4.445" dx="2" dy="0.6" layer="1" />
              <wire x1="-3.75" y1="5.2" x2="3.75" y2="5.2" width="0.127" layer="21" />
              <wire x1="3.75" y1="5.2" x2="3.75" y2="-5.2" width="0.127" layer="21" />
              <wire x1="3.75" y1="-5.2" x2="-3.75" y2="-5.2" width="0.127" layer="21" />
              <wire x1="-3.75" y1="-5.2" x2="-3.75" y2="5.2" width="0.127" layer="21" />
              <text x="-3.75" y="5.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="HDR-1X2">
              <description>Pin header, 2.54 mm pitch, 1.0 mm drill.</description>
              <pad name="1" x="-1.27" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="1.27" y="0" drill="1" diameter="1.8" />
              <wire x1="-2.54" y1="1.27" x2="2.54" y2="1.27" width="0.127" layer="21" />
              <wire x1="2.54" y1="1.27" x2="2.54" y2="-1.27" width="0.127" layer="21" />
              <wire x1="2.54" y1="-1.27" x2="-2.54" y2="-1.27" width="0.127" layer="21" />
              <wire x1="-2.54" y1="-1.27" x2="-2.54" y2="1.27" width="0.127" layer="21" />
              <text x="-2.54" y="1.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="HDR-1X3">
              <description>Pin header, 2.54 mm pitch, 1.0 mm drill.</description>
              <pad name="1" x="-2.54" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="0" y="0" drill="1" diameter="1.8" />
              <pad name="3" x="2.54" y="0" drill="1" diameter="1.8" />
              <wire x1="-3.81" y1="1.27" x2="3.81" y2="1.27" width="0.127" layer="21" />
              <wire x1="3.81" y1="1.27" x2="3.81" y2="-1.27" width="0.127" layer="21" />
              <wire x1="3.81" y1="-1.27" x2="-3.81" y2="-1.27" width="0.127" layer="21" />
              <wire x1="-3.81" y1="-1.27" x2="-3.81" y2="1.27" width="0.127" layer="21" />
              <text x="-3.81" y="1.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="HDR-1X5">
              <description>Pin header, 2.54 mm pitch, 1.0 mm drill.</description>
              <pad name="1" x="-5.08" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="-2.54" y="0" drill="1" diameter="1.8" />
              <pad name="3" x="0" y="0" drill="1" diameter="1.8" />
              <pad name="4" x="2.54" y="0" drill="1" diameter="1.8" />
              <pad name="5" x="5.08" y="0" drill="1" diameter="1.8" />
              <wire x1="-6.35" y1="1.27" x2="6.35" y2="1.27" width="0.127" layer="21" />
              <wire x1="6.35" y1="1.27" x2="6.35" y2="-1.27" width="0.127" layer="21" />
              <wire x1="6.35" y1="-1.27" x2="-6.35" y2="-1.27" width="0.127" layer="21" />
              <wire x1="-6.35" y1="-1.27" x2="-6.35" y2="1.27" width="0.127" layer="21" />
              <text x="-6.35" y="1.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="MOLEX-43045-0412">
              <description>Molex Micro-Fit 3.0 vertical header, 2x2, from sales drawing SD-43045-005. Check against the current Molex drawing before ordering boards.</description>
              <pad name="1" x="1.5" y="-1.5" drill="1.02" diameter="1.9" shape="square" />
              <pad name="2" x="-1.5" y="-1.5" drill="1.02" diameter="1.9" />
              <pad name="3" x="1.5" y="1.5" drill="1.02" diameter="1.9" />
              <pad name="4" x="-1.5" y="1.5" drill="1.02" diameter="1.9" />
              <hole x="-4.5" y="2.44" drill="1.02" />
              <hole x="4.5" y="2.44" drill="1.02" />
              <wire x1="-4.825" y1="3.685" x2="4.825" y2="3.685" width="0.127" layer="21" />
              <wire x1="4.825" y1="3.685" x2="4.825" y2="-3.685" width="0.127" layer="21" />
              <wire x1="4.825" y1="-3.685" x2="-4.825" y2="-3.685" width="0.127" layer="21" />
              <wire x1="-4.825" y1="-3.685" x2="-4.825" y2="3.685" width="0.127" layer="21" />
              <text x="-4.825" y="4.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="TESTPAD">
              <description>1.5 mm round test pad.</description>
              <smd name="1" x="0" y="0" dx="1.5" dy="1.5" layer="1" roundness="100" />
              <text x="-1" y="1.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="OLIMEX-ESP32-P4-DEVKIT">
              <description>Top view, USB-C at the bottom, Ethernet jack at the top (it overhangs the edge). The DevKit carries parts on its underside: keep the carrier clear beneath it, or seat it on sockets.</description>
              <pad name="EXT1_1" x="-12.7" y="-24.13" drill="1" diameter="1.8" shape="square" />
              <pad name="EXT2_1" x="12.7" y="-24.13" drill="1" diameter="1.8" shape="square" />
              <pad name="EXT1_2" x="-12.7" y="-21.59" drill="1" diameter="1.8" />
              <pad name="EXT2_2" x="12.7" y="-21.59" drill="1" diameter="1.8" />
              <pad name="EXT1_3" x="-12.7" y="-19.05" drill="1" diameter="1.8" />
              <pad name="EXT2_3" x="12.7" y="-19.05" drill="1" diameter="1.8" />
              <pad name="EXT1_4" x="-12.7" y="-16.51" drill="1" diameter="1.8" />
              <pad name="EXT2_4" x="12.7" y="-16.51" drill="1" diameter="1.8" />
              <pad name="EXT1_5" x="-12.7" y="-13.97" drill="1" diameter="1.8" />
              <pad name="EXT2_5" x="12.7" y="-13.97" drill="1" diameter="1.8" />
              <pad name="EXT1_6" x="-12.7" y="-11.43" drill="1" diameter="1.8" />
              <pad name="EXT2_6" x="12.7" y="-11.43" drill="1" diameter="1.8" />
              <pad name="EXT1_7" x="-12.7" y="-8.89" drill="1" diameter="1.8" />
              <pad name="EXT2_7" x="12.7" y="-8.89" drill="1" diameter="1.8" />
              <pad name="EXT1_8" x="-12.7" y="-6.35" drill="1" diameter="1.8" />
              <pad name="EXT2_8" x="12.7" y="-6.35" drill="1" diameter="1.8" />
              <pad name="EXT1_9" x="-12.7" y="-3.81" drill="1" diameter="1.8" />
              <pad name="EXT2_9" x="12.7" y="-3.81" drill="1" diameter="1.8" />
              <pad name="EXT1_10" x="-12.7" y="-1.27" drill="1" diameter="1.8" />
              <pad name="EXT2_10" x="12.7" y="-1.27" drill="1" diameter="1.8" />
              <pad name="EXT1_11" x="-12.7" y="1.27" drill="1" diameter="1.8" />
              <pad name="EXT2_11" x="12.7" y="1.27" drill="1" diameter="1.8" />
              <pad name="EXT1_12" x="-12.7" y="3.81" drill="1" diameter="1.8" />
              <pad name="EXT2_12" x="12.7" y="3.81" drill="1" diameter="1.8" />
              <pad name="EXT1_13" x="-12.7" y="6.35" drill="1" diameter="1.8" />
              <pad name="EXT2_13" x="12.7" y="6.35" drill="1" diameter="1.8" />
              <pad name="EXT1_14" x="-12.7" y="8.89" drill="1" diameter="1.8" />
              <pad name="EXT2_14" x="12.7" y="8.89" drill="1" diameter="1.8" />
              <pad name="EXT1_15" x="-12.7" y="11.43" drill="1" diameter="1.8" />
              <pad name="EXT2_15" x="12.7" y="11.43" drill="1" diameter="1.8" />
              <pad name="EXT1_16" x="-12.7" y="13.97" drill="1" diameter="1.8" />
              <pad name="EXT2_16" x="12.7" y="13.97" drill="1" diameter="1.8" />
              <pad name="EXT1_17" x="-12.7" y="16.51" drill="1" diameter="1.8" />
              <pad name="EXT2_17" x="12.7" y="16.51" drill="1" diameter="1.8" />
              <pad name="EXT1_18" x="-12.7" y="19.05" drill="1" diameter="1.8" />
              <pad name="EXT2_18" x="12.7" y="19.05" drill="1" diameter="1.8" />
              <pad name="EXT1_19" x="-12.7" y="21.59" drill="1" diameter="1.8" />
              <pad name="EXT2_19" x="12.7" y="21.59" drill="1" diameter="1.8" />
              <pad name="EXT1_20" x="-12.7" y="24.13" drill="1" diameter="1.8" />
              <pad name="EXT2_20" x="12.7" y="24.13" drill="1" diameter="1.8" />
              <hole x="-11.5" y="31.5" drill="3.3" />
              <hole x="11.5" y="31.5" drill="3.3" />
              <hole x="-11.5" y="-33.5" drill="3.3" />
              <hole x="11.5" y="-33.5" drill="3.3" />
              <wire x1="-15" y1="35" x2="15" y2="35" width="0.127" layer="21" />
              <wire x1="15" y1="35" x2="15" y2="-37" width="0.127" layer="21" />
              <wire x1="15" y1="-37" x2="-15" y2="-37" width="0.127" layer="21" />
              <wire x1="-15" y1="-37" x2="-15" y2="35" width="0.127" layer="21" />
              <text x="-15" y="35.5" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="ESP32-C6-MINI-1-PLACEHOLDER">
              <description>PLACEHOLDER with the module's 53 pad names. Use the ESP32-C6-MINI-1 footprint from the LCSC part (C5736265) or Espressif's library. 13.2 x 16.6 mm.</description>
              <smd name="1" x="-6.2" y="4.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="13" x="-4.4" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="25" x="6.2" y="-4.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="37" x="4.4" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="2" x="-6.2" y="3.6" dx="0.8" dy="0.4" layer="1" />
              <smd name="14" x="-3.6" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="26" x="6.2" y="-3.6" dx="0.8" dy="0.4" layer="1" />
              <smd name="38" x="3.6" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="3" x="-6.2" y="2.8" dx="0.8" dy="0.4" layer="1" />
              <smd name="15" x="-2.8" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="27" x="6.2" y="-2.8" dx="0.8" dy="0.4" layer="1" />
              <smd name="39" x="2.8" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="4" x="-6.2" y="2" dx="0.8" dy="0.4" layer="1" />
              <smd name="16" x="-2" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="28" x="6.2" y="-2" dx="0.8" dy="0.4" layer="1" />
              <smd name="40" x="2" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="5" x="-6.2" y="1.2" dx="0.8" dy="0.4" layer="1" />
              <smd name="17" x="-1.2" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="29" x="6.2" y="-1.2" dx="0.8" dy="0.4" layer="1" />
              <smd name="41" x="1.2" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="6" x="-6.2" y="0.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="18" x="-0.4" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="30" x="6.2" y="-0.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="42" x="0.4" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="7" x="-6.2" y="-0.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="19" x="0.4" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="31" x="6.2" y="0.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="43" x="-0.4" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="8" x="-6.2" y="-1.2" dx="0.8" dy="0.4" layer="1" />
              <smd name="20" x="1.2" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="32" x="6.2" y="1.2" dx="0.8" dy="0.4" layer="1" />
              <smd name="44" x="-1.2" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="9" x="-6.2" y="-2" dx="0.8" dy="0.4" layer="1" />
              <smd name="21" x="2" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="33" x="6.2" y="2" dx="0.8" dy="0.4" layer="1" />
              <smd name="45" x="-2" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="10" x="-6.2" y="-2.8" dx="0.8" dy="0.4" layer="1" />
              <smd name="22" x="2.8" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="34" x="6.2" y="2.8" dx="0.8" dy="0.4" layer="1" />
              <smd name="46" x="-2.8" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="11" x="-6.2" y="-3.6" dx="0.8" dy="0.4" layer="1" />
              <smd name="23" x="3.6" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="35" x="6.2" y="3.6" dx="0.8" dy="0.4" layer="1" />
              <smd name="47" x="-3.6" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="12" x="-6.2" y="-4.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="24" x="4.4" y="-7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="36" x="6.2" y="4.4" dx="0.8" dy="0.4" layer="1" />
              <smd name="48" x="-4.4" y="7.9" dx="0.4" dy="0.8" layer="1" />
              <smd name="49" x="-2.4" y="0" dx="0.8" dy="0.8" layer="1" />
              <smd name="50" x="-1.2" y="0" dx="0.8" dy="0.8" layer="1" />
              <smd name="51" x="0" y="0" dx="0.8" dy="0.8" layer="1" />
              <smd name="52" x="1.2" y="0" dx="0.8" dy="0.8" layer="1" />
              <smd name="53" x="2.4" y="0" dx="0.8" dy="0.8" layer="1" />
              <wire x1="-6.6" y1="8.3" x2="6.6" y2="8.3" width="0.127" layer="21" />
              <wire x1="6.6" y1="8.3" x2="6.6" y2="-8.3" width="0.127" layer="21" />
              <wire x1="6.6" y1="-8.3" x2="-6.6" y2="-8.3" width="0.127" layer="21" />
              <wire x1="-6.6" y1="-8.3" x2="-6.6" y2="8.3" width="0.127" layer="21" />
              <text x="-6.6" y="8.8" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="CR2032-HOLDER-PLACEHOLDER">
              <description>PLACEHOLDER. Replace with the footprint of the 2032 holder you buy.</description>
              <pad name="POS" x="-10.25" y="0" drill="1.2" diameter="2.4" shape="square" />
              <pad name="NEG" x="10.25" y="0" drill="1.2" diameter="2.4" />
              <wire x1="-11.5" y1="8" x2="11.5" y2="8" width="0.127" layer="21" />
              <wire x1="11.5" y1="8" x2="11.5" y2="-8" width="0.127" layer="21" />
              <wire x1="11.5" y1="-8" x2="-11.5" y2="-8" width="0.127" layer="21" />
              <wire x1="-11.5" y1="-8" x2="-11.5" y2="8" width="0.127" layer="21" />
              <text x="-11.5" y="8.5" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="POLOLU-D36V28F-PLACEHOLDER">
              <description>PLACEHOLDER. Replace with the hole pattern from Pololu's D36V28Fx drill guide.</description>
              <pad name="VIN" x="-5.08" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="GND1" x="-2.54" y="0" drill="1" diameter="1.8" />
              <pad name="GND2" x="0" y="0" drill="1" diameter="1.8" />
              <pad name="VOUT" x="2.54" y="0" drill="1" diameter="1.8" />
              <pad name="EN" x="5.08" y="0" drill="1" diameter="1.8" />
              <wire x1="-8.9" y1="10.2" x2="8.9" y2="10.2" width="0.127" layer="21" />
              <wire x1="8.9" y1="10.2" x2="8.9" y2="-10.2" width="0.127" layer="21" />
              <wire x1="8.9" y1="-10.2" x2="-8.9" y2="-10.2" width="0.127" layer="21" />
              <wire x1="-8.9" y1="-10.2" x2="-8.9" y2="10.2" width="0.127" layer="21" />
              <text x="-8.9" y="10.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
          </packages>
          <symbols>
            <symbol name="BATTERY">
              <wire x1="-2.54" y1="0" x2="-0.635" y2="0" width="0.254" layer="94" />
              <wire x1="0.635" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <wire x1="-0.635" y1="-2.286" x2="-0.635" y2="2.286" width="0.254" layer="94" />
              <wire x1="0.635" y1="-1.016" x2="0.635" y2="1.016" width="0.254" layer="94" />
              <text x="-2.4" y="0.9" size="1.27" layer="94">+</text>
              <pin name="+" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="-" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="BUCK-MODULE">
              <wire x1="-10.16" y1="8.89" x2="10.16" y2="8.89" width="0.254" layer="94" />
              <wire x1="10.16" y1="8.89" x2="10.16" y2="-8.89" width="0.254" layer="94" />
              <wire x1="10.16" y1="-8.89" x2="-10.16" y2="-8.89" width="0.254" layer="94" />
              <wire x1="-10.16" y1="-8.89" x2="-10.16" y2="8.89" width="0.254" layer="94" />
              <pin name="VIN" x="-15.24" y="5.08" visible="pin" length="middle" direction="pas" />
              <pin name="EN" x="-15.24" y="0" visible="pin" length="middle" direction="pas" />
              <pin name="GND@1" x="-15.24" y="-5.08" visible="pin" length="middle" direction="pas" />
              <pin name="VOUT" x="15.24" y="5.08" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@2" x="15.24" y="0" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-10.16" y="9.89" size="1.778" layer="95">&gt;NAME</text>
              <text x="-10.16" y="-11.69" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="CAPACITOR">
              <wire x1="-2.54" y1="0" x2="-0.635" y2="0" width="0.254" layer="94" />
              <wire x1="0.635" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <wire x1="-0.635" y1="-1.778" x2="-0.635" y2="1.778" width="0.254" layer="94" />
              <wire x1="0.635" y1="-1.778" x2="0.635" y2="1.778" width="0.254" layer="94" />
              <pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="CAPACITOR-POL">
              <wire x1="-2.54" y1="0" x2="-0.635" y2="0" width="0.254" layer="94" />
              <wire x1="0.635" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <wire x1="-0.635" y1="-1.778" x2="-0.635" y2="1.778" width="0.254" layer="94" />
              <wire x1="0.635" y1="-1.778" x2="0.635" y2="1.778" width="0.254" layer="94" />
              <text x="-2.4" y="0.6" size="1.27" layer="94">+</text>
              <pin name="+" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="-" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="DIODE">
              <wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.254" layer="94" />
              <wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94" />
              <wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94" />
              <wire x1="1.27" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94" />
              <wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94" />
              <pin name="A" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="K" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="DS3231SN">
              <wire x1="-12.7" y1="21.59" x2="12.7" y2="21.59" width="0.254" layer="94" />
              <wire x1="12.7" y1="21.59" x2="12.7" y2="-21.59" width="0.254" layer="94" />
              <wire x1="12.7" y1="-21.59" x2="-12.7" y2="-21.59" width="0.254" layer="94" />
              <wire x1="-12.7" y1="-21.59" x2="-12.7" y2="21.59" width="0.254" layer="94" />
              <pin name="VCC" x="-17.78" y="17.78" visible="pin" length="middle" direction="pas" />
              <pin name="VBAT" x="-17.78" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="SDA" x="-17.78" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="SCL" x="-17.78" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="INT_SQW" x="-17.78" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="RST" x="-17.78" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="32KHZ" x="-17.78" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-17.78" y="-17.78" visible="pin" length="middle" direction="pas" />
              <pin name="NC@5" x="17.78" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@6" x="17.78" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@7" x="17.78" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@8" x="17.78" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@9" x="17.78" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@10" x="17.78" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@11" x="17.78" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@12" x="17.78" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-12.7" y="22.59" size="1.778" layer="95">&gt;NAME</text>
              <text x="-12.7" y="-24.39" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="ESP32-C6-MINI-1">
              <wire x1="-12.7" y1="35.56" x2="12.7" y2="35.56" width="0.254" layer="94" />
              <wire x1="12.7" y1="35.56" x2="12.7" y2="-35.56" width="0.254" layer="94" />
              <wire x1="12.7" y1="-35.56" x2="-12.7" y2="-35.56" width="0.254" layer="94" />
              <wire x1="-12.7" y1="-35.56" x2="-12.7" y2="35.56" width="0.254" layer="94" />
              <pin name="3V3" x="-17.78" y="33.02" visible="pin" length="middle" direction="pas" />
              <pin name="EN" x="-17.78" y="30.48" visible="pin" length="middle" direction="pas" />
              <pin name="IO8" x="-17.78" y="27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO9" x="-17.78" y="25.4" visible="pin" length="middle" direction="pas" />
              <pin name="RXD0" x="-17.78" y="22.86" visible="pin" length="middle" direction="pas" />
              <pin name="TXD0" x="-17.78" y="20.32" visible="pin" length="middle" direction="pas" />
              <pin name="IO2" x="-17.78" y="17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO18" x="-17.78" y="15.24" visible="pin" length="middle" direction="pas" />
              <pin name="IO19" x="-17.78" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO20" x="-17.78" y="10.16" visible="pin" length="middle" direction="pas" />
              <pin name="IO21" x="-17.78" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO22" x="-17.78" y="5.08" visible="pin" length="middle" direction="pas" />
              <pin name="IO23" x="-17.78" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO0" x="-17.78" y="0" visible="pin" length="middle" direction="pas" />
              <pin name="IO1" x="-17.78" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO3" x="-17.78" y="-5.08" visible="pin" length="middle" direction="pas" />
              <pin name="IO4" x="-17.78" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO5" x="-17.78" y="-10.16" visible="pin" length="middle" direction="pas" />
              <pin name="IO6" x="-17.78" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO7" x="-17.78" y="-15.24" visible="pin" length="middle" direction="pas" />
              <pin name="IO12" x="-17.78" y="-17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO13" x="-17.78" y="-20.32" visible="pin" length="middle" direction="pas" />
              <pin name="IO14" x="-17.78" y="-22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO15" x="-17.78" y="-25.4" visible="pin" length="middle" direction="pas" />
              <pin name="NC@4" x="-17.78" y="-27.94" visible="pin" length="middle" direction="pas" />
              <pin name="NC@7" x="-17.78" y="-30.48" visible="pin" length="middle" direction="pas" />
              <pin name="NC@21" x="-17.78" y="-33.02" visible="pin" length="middle" direction="pas" />
              <pin name="NC@32" x="17.78" y="33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@33" x="17.78" y="30.48" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@34" x="17.78" y="27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@35" x="17.78" y="25.4" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@1" x="17.78" y="22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@2" x="17.78" y="20.32" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@11" x="17.78" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@14" x="17.78" y="15.24" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@36" x="17.78" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@37" x="17.78" y="10.16" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@38" x="17.78" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@39" x="17.78" y="5.08" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@40" x="17.78" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@41" x="17.78" y="0" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@42" x="17.78" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@43" x="17.78" y="-5.08" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@44" x="17.78" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@45" x="17.78" y="-10.16" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@46" x="17.78" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@47" x="17.78" y="-15.24" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@48" x="17.78" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@49" x="17.78" y="-20.32" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@50" x="17.78" y="-22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@51" x="17.78" y="-25.4" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@52" x="17.78" y="-27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@53" x="17.78" y="-30.48" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-12.7" y="36.56" size="1.778" layer="95">&gt;NAME</text>
              <text x="-12.7" y="-38.36" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="FUSE">
              <wire x1="-2.54" y1="-0.889" x2="2.54" y2="-0.889" width="0.254" layer="94" />
              <wire x1="2.54" y1="-0.889" x2="2.54" y2="0.889" width="0.254" layer="94" />
              <wire x1="2.54" y1="0.889" x2="-2.54" y2="0.889" width="0.254" layer="94" />
              <wire x1="-2.54" y1="0.889" x2="-2.54" y2="-0.889" width="0.254" layer="94" />
              <wire x1="-2.54" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="GPS-HEADER">
              <wire x1="-7.62" y1="13.97" x2="7.62" y2="13.97" width="0.254" layer="94" />
              <wire x1="7.62" y1="13.97" x2="7.62" y2="-13.97" width="0.254" layer="94" />
              <wire x1="7.62" y1="-13.97" x2="-7.62" y2="-13.97" width="0.254" layer="94" />
              <wire x1="-7.62" y1="-13.97" x2="-7.62" y2="13.97" width="0.254" layer="94" />
              <pin name="VCC" x="-12.7" y="10.16" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-12.7" y="5.08" visible="pin" length="middle" direction="pas" />
              <pin name="TXD" x="-12.7" y="0" visible="pin" length="middle" direction="pas" />
              <pin name="RXD" x="-12.7" y="-5.08" visible="pin" length="middle" direction="pas" />
              <pin name="PPS" x="-12.7" y="-10.16" visible="pin" length="middle" direction="pas" />
              <text x="-7.62" y="14.97" size="1.778" layer="95">&gt;NAME</text>
              <text x="-7.62" y="-16.77" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="JUMPER">
              <circle x="-1.27" y="0" radius="0.635" width="0.254" layer="94" />
              <circle x="1.27" y="0" radius="0.635" width="0.254" layer="94" />
              <wire x1="-2.54" y1="0" x2="-1.905" y2="0" width="0.254" layer="94" />
              <wire x1="1.905" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="JUMPER-3">
              <wire x1="-5.08" y1="8.89" x2="5.08" y2="8.89" width="0.254" layer="94" />
              <wire x1="5.08" y1="8.89" x2="5.08" y2="-8.89" width="0.254" layer="94" />
              <wire x1="5.08" y1="-8.89" x2="-5.08" y2="-8.89" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-8.89" x2="-5.08" y2="8.89" width="0.254" layer="94" />
              <pin name="1" x="-10.16" y="5.08" visible="pin" length="middle" direction="pas" />
              <pin name="2" x="-10.16" y="0" visible="pin" length="middle" direction="pas" />
              <pin name="3" x="-10.16" y="-5.08" visible="pin" length="middle" direction="pas" />
              <text x="-5.08" y="9.89" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-11.69" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="MICROFIT-2X2">
              <wire x1="-5.08" y1="11.43" x2="5.08" y2="11.43" width="0.254" layer="94" />
              <wire x1="5.08" y1="11.43" x2="5.08" y2="-11.43" width="0.254" layer="94" />
              <wire x1="5.08" y1="-11.43" x2="-5.08" y2="-11.43" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-11.43" x2="-5.08" y2="11.43" width="0.254" layer="94" />
              <pin name="1" x="-10.16" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="2" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="3" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="4" x="-10.16" y="-7.62" visible="pin" length="middle" direction="pas" />
              <text x="-5.08" y="12.43" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-14.23" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="OLIMEX-ESP32-P4-DEVKIT">
              <wire x1="-15.24" y1="52.07" x2="15.24" y2="52.07" width="0.254" layer="94" />
              <wire x1="15.24" y1="52.07" x2="15.24" y2="-52.07" width="0.254" layer="94" />
              <wire x1="15.24" y1="-52.07" x2="-15.24" y2="-52.07" width="0.254" layer="94" />
              <wire x1="-15.24" y1="-52.07" x2="-15.24" y2="52.07" width="0.254" layer="94" />
              <pin name="IO19" x="-20.32" y="48.26" visible="pin" length="middle" direction="pas" />
              <pin name="IO18" x="-20.32" y="43.18" visible="pin" length="middle" direction="pas" />
              <pin name="IO17" x="-20.32" y="38.1" visible="pin" length="middle" direction="pas" />
              <pin name="IO16" x="-20.32" y="33.02" visible="pin" length="middle" direction="pas" />
              <pin name="IO15" x="-20.32" y="27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO14" x="-20.32" y="22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO13" x="-20.32" y="17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO12" x="-20.32" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO11" x="-20.32" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO10" x="-20.32" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO9" x="-20.32" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO8_SCL" x="-20.32" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO7_SDA" x="-20.32" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO6" x="-20.32" y="-17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO5" x="-20.32" y="-22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO4" x="-20.32" y="-27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO3_SD_DET" x="-20.32" y="-33.02" visible="pin" length="middle" direction="pas" />
              <pin name="IO2_LED" x="-20.32" y="-38.1" visible="pin" length="middle" direction="pas" />
              <pin name="GND@1" x="-20.32" y="-43.18" visible="pin" length="middle" direction="pas" />
              <pin name="3V3" x="-20.32" y="-48.26" visible="pin" length="middle" direction="pas" />
              <pin name="USB_DN" x="20.32" y="48.26" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="USB_DP" x="20.32" y="43.18" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@4" x="20.32" y="38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="USB1_N" x="20.32" y="33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="USB1_P" x="20.32" y="27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@3" x="20.32" y="22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="EN" x="20.32" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO20" x="20.32" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO21" x="20.32" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO22" x="20.32" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO23" x="20.32" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO32" x="20.32" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO33" x="20.32" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO46" x="20.32" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO47" x="20.32" y="-22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO48" x="20.32" y="-27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO53" x="20.32" y="-33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO54" x="20.32" y="-38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@2" x="20.32" y="-43.18" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="5V" x="20.32" y="-48.26" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-15.24" y="53.07" size="1.778" layer="95">&gt;NAME</text>
              <text x="-15.24" y="-54.87" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="PESD2CAN">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="IO1" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO2" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="POWER-2">
              <wire x1="-7.62" y1="6.35" x2="7.62" y2="6.35" width="0.254" layer="94" />
              <wire x1="7.62" y1="6.35" x2="7.62" y2="-6.35" width="0.254" layer="94" />
              <wire x1="7.62" y1="-6.35" x2="-7.62" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-7.62" y1="-6.35" x2="-7.62" y2="6.35" width="0.254" layer="94" />
              <pin name="+5V" x="-12.7" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-12.7" y="-2.54" visible="pin" length="middle" direction="pas" />
              <text x="-7.62" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-7.62" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="RESISTOR">
              <wire x1="-2.54" y1="-1.016" x2="2.54" y2="-1.016" width="0.254" layer="94" />
              <wire x1="2.54" y1="-1.016" x2="2.54" y2="1.016" width="0.254" layer="94" />
              <wire x1="2.54" y1="1.016" x2="-2.54" y2="1.016" width="0.254" layer="94" />
              <wire x1="-2.54" y1="1.016" x2="-2.54" y2="-1.016" width="0.254" layer="94" />
              <pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="SN65HVD230">
              <wire x1="-10.16" y1="16.51" x2="10.16" y2="16.51" width="0.254" layer="94" />
              <wire x1="10.16" y1="16.51" x2="10.16" y2="-16.51" width="0.254" layer="94" />
              <wire x1="10.16" y1="-16.51" x2="-10.16" y2="-16.51" width="0.254" layer="94" />
              <wire x1="-10.16" y1="-16.51" x2="-10.16" y2="16.51" width="0.254" layer="94" />
              <pin name="D" x="-15.24" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="R" x="-15.24" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="RS" x="-15.24" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VREF" x="-15.24" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VCC" x="-15.24" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-15.24" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="CANH" x="15.24" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="CANL" x="15.24" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-10.16" y="17.51" size="1.778" layer="95">&gt;NAME</text>
              <text x="-10.16" y="-19.31" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="TESTPOINT">
              <circle x="2.54" y="0" radius="0.9" width="0.254" layer="94" />
              <wire x1="0" y1="0" x2="1.64" y2="0" width="0.254" layer="94" />
              <pin name="TP" x="-2.54" y="0" visible="off" length="short" direction="pas" />
              <text x="0" y="1.6" size="1.778" layer="95">&gt;NAME</text>
              <text x="0" y="-3.4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="TJA1051T-3">
              <wire x1="-10.16" y1="16.51" x2="10.16" y2="16.51" width="0.254" layer="94" />
              <wire x1="10.16" y1="16.51" x2="10.16" y2="-16.51" width="0.254" layer="94" />
              <wire x1="10.16" y1="-16.51" x2="-10.16" y2="-16.51" width="0.254" layer="94" />
              <wire x1="-10.16" y1="-16.51" x2="-10.16" y2="16.51" width="0.254" layer="94" />
              <pin name="TXD" x="-15.24" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="RXD" x="-15.24" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="S" x="-15.24" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VIO" x="-15.24" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VCC" x="-15.24" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-15.24" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="CANH" x="15.24" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="CANL" x="15.24" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-10.16" y="17.51" size="1.778" layer="95">&gt;NAME</text>
              <text x="-10.16" y="-19.31" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="TVS-BIDIR">
              <wire x1="-2.54" y1="0" x2="-1.778" y2="0" width="0.254" layer="94" />
              <wire x1="1.778" y1="0" x2="2.54" y2="0" width="0.254" layer="94" />
              <wire x1="-1.778" y1="1.27" x2="-1.778" y2="-1.27" width="0.254" layer="94" />
              <wire x1="-1.778" y1="-1.27" x2="0" y2="0" width="0.254" layer="94" />
              <wire x1="0" y1="0" x2="-1.778" y2="1.27" width="0.254" layer="94" />
              <wire x1="1.778" y1="1.27" x2="1.778" y2="-1.27" width="0.254" layer="94" />
              <wire x1="1.778" y1="-1.27" x2="0" y2="0" width="0.254" layer="94" />
              <wire x1="0" y1="0" x2="1.778" y2="1.27" width="0.254" layer="94" />
              <wire x1="0" y1="1.524" x2="0" y2="-1.524" width="0.254" layer="94" />
              <pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" />
              <pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180" />
              <text x="-2.54" y="2.2" size="1.778" layer="95">&gt;NAME</text>
              <text x="-2.54" y="-4" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
          </symbols>
          <devicesets>
            <deviceset name="BAT54" prefix="D" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="DIODE" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="A" pad="1" />
                    <connect gate="G$1" pin="K" pad="3" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="BATTERY-2032" prefix="BT" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="BATTERY" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="CR2032-HOLDER-PLACEHOLDER">
                  <connects>
                    <connect gate="G$1" pin="+" pad="POS" />
                    <connect gate="G$1" pin="-" pad="NEG" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="C0805" prefix="C" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="CAPACITOR" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="0805">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="CPOL-8X10" prefix="C" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="CAPACITOR-POL" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="CAP-SMD-8X10">
                  <connects>
                    <connect gate="G$1" pin="+" pad="1" />
                    <connect gate="G$1" pin="-" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="DS3231SN" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="DS3231SN" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOIC16W">
                  <connects>
                    <connect gate="G$1" pin="32KHZ" pad="1" />
                    <connect gate="G$1" pin="VCC" pad="2" />
                    <connect gate="G$1" pin="INT_SQW" pad="3" />
                    <connect gate="G$1" pin="RST" pad="4" />
                    <connect gate="G$1" pin="GND" pad="13" />
                    <connect gate="G$1" pin="VBAT" pad="14" />
                    <connect gate="G$1" pin="SDA" pad="15" />
                    <connect gate="G$1" pin="SCL" pad="16" />
                    <connect gate="G$1" pin="NC@5" pad="5" />
                    <connect gate="G$1" pin="NC@6" pad="6" />
                    <connect gate="G$1" pin="NC@7" pad="7" />
                    <connect gate="G$1" pin="NC@8" pad="8" />
                    <connect gate="G$1" pin="NC@9" pad="9" />
                    <connect gate="G$1" pin="NC@10" pad="10" />
                    <connect gate="G$1" pin="NC@11" pad="11" />
                    <connect gate="G$1" pin="NC@12" pad="12" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="ESP32-C6-MINI-1" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="ESP32-C6-MINI-1" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="ESP32-C6-MINI-1-PLACEHOLDER">
                  <connects>
                    <connect gate="G$1" pin="3V3" pad="3" />
                    <connect gate="G$1" pin="EN" pad="8" />
                    <connect gate="G$1" pin="IO8" pad="22" />
                    <connect gate="G$1" pin="IO9" pad="23" />
                    <connect gate="G$1" pin="RXD0" pad="30" />
                    <connect gate="G$1" pin="TXD0" pad="31" />
                    <connect gate="G$1" pin="IO2" pad="5" />
                    <connect gate="G$1" pin="IO18" pad="24" />
                    <connect gate="G$1" pin="IO19" pad="25" />
                    <connect gate="G$1" pin="IO20" pad="26" />
                    <connect gate="G$1" pin="IO21" pad="27" />
                    <connect gate="G$1" pin="IO22" pad="28" />
                    <connect gate="G$1" pin="IO23" pad="29" />
                    <connect gate="G$1" pin="IO0" pad="12" />
                    <connect gate="G$1" pin="IO1" pad="13" />
                    <connect gate="G$1" pin="IO3" pad="6" />
                    <connect gate="G$1" pin="IO4" pad="9" />
                    <connect gate="G$1" pin="IO5" pad="10" />
                    <connect gate="G$1" pin="IO6" pad="15" />
                    <connect gate="G$1" pin="IO7" pad="16" />
                    <connect gate="G$1" pin="IO12" pad="17" />
                    <connect gate="G$1" pin="IO13" pad="18" />
                    <connect gate="G$1" pin="IO14" pad="19" />
                    <connect gate="G$1" pin="IO15" pad="20" />
                    <connect gate="G$1" pin="NC@4" pad="4" />
                    <connect gate="G$1" pin="NC@7" pad="7" />
                    <connect gate="G$1" pin="NC@21" pad="21" />
                    <connect gate="G$1" pin="NC@32" pad="32" />
                    <connect gate="G$1" pin="NC@33" pad="33" />
                    <connect gate="G$1" pin="NC@34" pad="34" />
                    <connect gate="G$1" pin="NC@35" pad="35" />
                    <connect gate="G$1" pin="GND@1" pad="1" />
                    <connect gate="G$1" pin="GND@2" pad="2" />
                    <connect gate="G$1" pin="GND@11" pad="11" />
                    <connect gate="G$1" pin="GND@14" pad="14" />
                    <connect gate="G$1" pin="GND@36" pad="36" />
                    <connect gate="G$1" pin="GND@37" pad="37" />
                    <connect gate="G$1" pin="GND@38" pad="38" />
                    <connect gate="G$1" pin="GND@39" pad="39" />
                    <connect gate="G$1" pin="GND@40" pad="40" />
                    <connect gate="G$1" pin="GND@41" pad="41" />
                    <connect gate="G$1" pin="GND@42" pad="42" />
                    <connect gate="G$1" pin="GND@43" pad="43" />
                    <connect gate="G$1" pin="GND@44" pad="44" />
                    <connect gate="G$1" pin="GND@45" pad="45" />
                    <connect gate="G$1" pin="GND@46" pad="46" />
                    <connect gate="G$1" pin="GND@47" pad="47" />
                    <connect gate="G$1" pin="GND@48" pad="48" />
                    <connect gate="G$1" pin="GND@49" pad="49" />
                    <connect gate="G$1" pin="GND@50" pad="50" />
                    <connect gate="G$1" pin="GND@51" pad="51" />
                    <connect gate="G$1" pin="GND@52" pad="52" />
                    <connect gate="G$1" pin="GND@53" pad="53" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="GPS-HEADER" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="GPS-HEADER" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X5">
                  <connects>
                    <connect gate="G$1" pin="VCC" pad="1" />
                    <connect gate="G$1" pin="GND" pad="2" />
                    <connect gate="G$1" pin="TXD" pad="3" />
                    <connect gate="G$1" pin="RXD" pad="4" />
                    <connect gate="G$1" pin="PPS" pad="5" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="JUMPER-2" prefix="JP" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="JUMPER" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X2">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="JUMPER-3" prefix="JP" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="JUMPER-3" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X3">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                    <connect gate="G$1" pin="3" pad="3" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="MICROFIT-2X2" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="MICROFIT-2X2" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="MOLEX-43045-0412">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                    <connect gate="G$1" pin="3" pad="3" />
                    <connect gate="G$1" pin="4" pad="4" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="OLIMEX-ESP32-P4-DEVKIT" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="OLIMEX-ESP32-P4-DEVKIT" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="OLIMEX-ESP32-P4-DEVKIT">
                  <connects>
                    <connect gate="G$1" pin="3V3" pad="EXT1_1" />
                    <connect gate="G$1" pin="GND@1" pad="EXT1_2" />
                    <connect gate="G$1" pin="IO2_LED" pad="EXT1_3" />
                    <connect gate="G$1" pin="IO3_SD_DET" pad="EXT1_4" />
                    <connect gate="G$1" pin="IO4" pad="EXT1_5" />
                    <connect gate="G$1" pin="IO5" pad="EXT1_6" />
                    <connect gate="G$1" pin="IO6" pad="EXT1_7" />
                    <connect gate="G$1" pin="IO7_SDA" pad="EXT1_8" />
                    <connect gate="G$1" pin="IO8_SCL" pad="EXT1_9" />
                    <connect gate="G$1" pin="IO9" pad="EXT1_10" />
                    <connect gate="G$1" pin="IO10" pad="EXT1_11" />
                    <connect gate="G$1" pin="IO11" pad="EXT1_12" />
                    <connect gate="G$1" pin="IO12" pad="EXT1_13" />
                    <connect gate="G$1" pin="IO13" pad="EXT1_14" />
                    <connect gate="G$1" pin="IO14" pad="EXT1_15" />
                    <connect gate="G$1" pin="IO15" pad="EXT1_16" />
                    <connect gate="G$1" pin="IO16" pad="EXT1_17" />
                    <connect gate="G$1" pin="IO17" pad="EXT1_18" />
                    <connect gate="G$1" pin="IO18" pad="EXT1_19" />
                    <connect gate="G$1" pin="IO19" pad="EXT1_20" />
                    <connect gate="G$1" pin="5V" pad="EXT2_1" />
                    <connect gate="G$1" pin="GND@2" pad="EXT2_2" />
                    <connect gate="G$1" pin="IO54" pad="EXT2_3" />
                    <connect gate="G$1" pin="IO53" pad="EXT2_4" />
                    <connect gate="G$1" pin="IO48" pad="EXT2_5" />
                    <connect gate="G$1" pin="IO47" pad="EXT2_6" />
                    <connect gate="G$1" pin="IO46" pad="EXT2_7" />
                    <connect gate="G$1" pin="IO33" pad="EXT2_8" />
                    <connect gate="G$1" pin="IO32" pad="EXT2_9" />
                    <connect gate="G$1" pin="IO23" pad="EXT2_10" />
                    <connect gate="G$1" pin="IO22" pad="EXT2_11" />
                    <connect gate="G$1" pin="IO21" pad="EXT2_12" />
                    <connect gate="G$1" pin="IO20" pad="EXT2_13" />
                    <connect gate="G$1" pin="EN" pad="EXT2_14" />
                    <connect gate="G$1" pin="GND@3" pad="EXT2_15" />
                    <connect gate="G$1" pin="USB1_P" pad="EXT2_16" />
                    <connect gate="G$1" pin="USB1_N" pad="EXT2_17" />
                    <connect gate="G$1" pin="GND@4" pad="EXT2_18" />
                    <connect gate="G$1" pin="USB_DP" pad="EXT2_19" />
                    <connect gate="G$1" pin="USB_DN" pad="EXT2_20" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="PESD2CAN" prefix="D" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="PESD2CAN" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="IO1" pad="1" />
                    <connect gate="G$1" pin="IO2" pad="2" />
                    <connect gate="G$1" pin="GND" pad="3" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="POLOLU-D36V28F5" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="BUCK-MODULE" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="POLOLU-D36V28F-PLACEHOLDER">
                  <connects>
                    <connect gate="G$1" pin="VIN" pad="VIN" />
                    <connect gate="G$1" pin="EN" pad="EN" />
                    <connect gate="G$1" pin="GND@1" pad="GND1" />
                    <connect gate="G$1" pin="VOUT" pad="VOUT" />
                    <connect gate="G$1" pin="GND@2" pad="GND2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="POLYFUSE-1812" prefix="F" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="FUSE" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="1812">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="POWER-2" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="POWER-2" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X2">
                  <connects>
                    <connect gate="G$1" pin="+5V" pad="1" />
                    <connect gate="G$1" pin="GND" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="R0805" prefix="R" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="RESISTOR" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="0805">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="R1206" prefix="R" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="RESISTOR" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="1206">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="SCHOTTKY-SMA" prefix="D" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="DIODE" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SMA">
                  <connects>
                    <connect gate="G$1" pin="K" pad="1" />
                    <connect gate="G$1" pin="A" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="SN65HVD230" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="SN65HVD230" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOIC8">
                  <connects>
                    <connect gate="G$1" pin="D" pad="1" />
                    <connect gate="G$1" pin="GND" pad="2" />
                    <connect gate="G$1" pin="VCC" pad="3" />
                    <connect gate="G$1" pin="R" pad="4" />
                    <connect gate="G$1" pin="VREF" pad="5" />
                    <connect gate="G$1" pin="CANL" pad="6" />
                    <connect gate="G$1" pin="CANH" pad="7" />
                    <connect gate="G$1" pin="RS" pad="8" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="TESTPOINT" prefix="TP" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="TESTPOINT" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="TESTPAD">
                  <connects>
                    <connect gate="G$1" pin="TP" pad="1" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="TJA1051T-3" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="TJA1051T-3" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOIC8">
                  <connects>
                    <connect gate="G$1" pin="TXD" pad="1" />
                    <connect gate="G$1" pin="GND" pad="2" />
                    <connect gate="G$1" pin="VCC" pad="3" />
                    <connect gate="G$1" pin="RXD" pad="4" />
                    <connect gate="G$1" pin="VIO" pad="5" />
                    <connect gate="G$1" pin="CANL" pad="6" />
                    <connect gate="G$1" pin="CANH" pad="7" />
                    <connect gate="G$1" pin="S" pad="8" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="TVS-SMB" prefix="D" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="TVS-BIDIR" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SMB">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
          </devicesets>
        </library>
      </libraries>
      <attributes />
      <variantdefs />
      <classes>
        <class number="0" name="default" width="0" drill="0" />
      </classes>
      <parts>
        <part name="J10" library="haltech-gauges" deviceset="MICROFIT-2X2" device="" value="43045-0412 ECU" />
        <part name="D1" library="haltech-gauges" deviceset="TVS-SMB" device="" value="SMBJ26CA">
          <attribute name="LCSC" value="C515606" />
        </part>
        <part name="D2" library="haltech-gauges" deviceset="SCHOTTKY-SMA" device="" value="SS34">
          <attribute name="LCSC" value="C8678" />
        </part>
        <part name="C1" library="haltech-gauges" deviceset="CPOL-8X10" device="" value="100u 50V">
          <attribute name="LCSC" value="C116241" />
        </part>
        <part name="C2" library="haltech-gauges" deviceset="C0805" device="" value="1u 50V">
          <attribute name="LCSC" value="C28323" />
        </part>
        <part name="U1" library="haltech-gauges" deviceset="POLOLU-D36V28F5" device="" value="D36V28F5" />
        <part name="C3" library="haltech-gauges" deviceset="CPOL-8X10" device="" value="100u 50V">
          <attribute name="LCSC" value="C116241" />
        </part>
        <part name="C4" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="C5" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="J13" library="haltech-gauges" deviceset="POWER-2" device="" value="TO DEVKIT POE_PWR1" />
        <part name="F2" library="haltech-gauges" deviceset="POLYFUSE-1812" device="" value="1812L200/16DR">
          <attribute name="LCSC" value="C439873" />
        </part>
        <part name="J11" library="haltech-gauges" deviceset="MICROFIT-2X2" device="" value="43045-0412 CHAIN" />
        <part name="U4" library="haltech-gauges" deviceset="OLIMEX-ESP32-P4-DEVKIT" device="" value="ESP32-P4-DevKit" />
        <part name="U2" library="haltech-gauges" deviceset="TJA1051T-3" device="" value="TJA1051T/3">
          <attribute name="LCSC" value="C38695" />
        </part>
        <part name="U3" library="haltech-gauges" deviceset="SN65HVD230" device="" value="SN65HVD230DR">
          <attribute name="LCSC" value="C12084" />
        </part>
        <part name="D4" library="haltech-gauges" deviceset="PESD2CAN" device="" value="PESD2CAN">
          <attribute name="LCSC" value="C75176" />
        </part>
        <part name="R3" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="JP1" library="haltech-gauges" deviceset="JUMPER-2" device="" value="ECU TERM" />
        <part name="D5" library="haltech-gauges" deviceset="PESD2CAN" device="" value="PESD2CAN">
          <attribute name="LCSC" value="C75176" />
        </part>
        <part name="R4" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="R5" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="R6" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="R7" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="C9" library="haltech-gauges" deviceset="C0805" device="" value="4n7">
          <attribute name="LCSC" value="C1744" />
        </part>
        <part name="J12" library="haltech-gauges" deviceset="GPS-HEADER" device="" value="GPS 1x5 2.54" />
        <part name="JP3" library="haltech-gauges" deviceset="JUMPER-3" device="" value="GPS 3V3/5V" />
        <part name="R22" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="R23" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="C16" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="U6" library="haltech-gauges" deviceset="ESP32-C6-MINI-1" device="" value="ESP32-C6-MINI-1-N4">
          <attribute name="LCSC" value="C5736265" />
        </part>
        <part name="R10" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="C13" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C11" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="C12" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="R11" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R12" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R13" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R14" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R15" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R16" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R17" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="TP6" library="haltech-gauges" deviceset="TESTPOINT" device="" value="C6 EN" />
        <part name="TP7" library="haltech-gauges" deviceset="TESTPOINT" device="" value="C6 TXD" />
        <part name="TP8" library="haltech-gauges" deviceset="TESTPOINT" device="" value="C6 RXD" />
        <part name="TP9" library="haltech-gauges" deviceset="TESTPOINT" device="" value="C6 BOOT" />
        <part name="C6" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C7" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C8" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C10" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="R1" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R2" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R8" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R9" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="U5" library="haltech-gauges" deviceset="DS3231SN" device="" value="DS3231SN#">
          <attribute name="LCSC" value="C9866" />
        </part>
        <part name="C14" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="R20" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="BT1" library="haltech-gauges" deviceset="BATTERY-2032" device="" value="CR2032 holder" />
        <part name="C15" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="D6" library="haltech-gauges" deviceset="BAT54" device="" value="BAT54">
          <attribute name="LCSC" value="C8590" />
        </part>
        <part name="R21" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="JP2" library="haltech-gauges" deviceset="JUMPER-2" device="" value="ML2032 CHARGE" />
        <part name="TP1" library="haltech-gauges" deviceset="TESTPOINT" device="" value="GND" />
        <part name="TP2" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+5V" />
        <part name="TP3" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+3V3" />
        <part name="TP4" library="haltech-gauges" deviceset="TESTPOINT" device="" value="VBAT" />
        <part name="TP5" library="haltech-gauges" deviceset="TESTPOINT" device="" value="RST" />
      </parts>
      <sheets>
        <sheet>
          <plain>
            <text x="10.16" y="438.48" size="2.54" layer="97">Haltech gauges: hub carrier, ESP32-P4 variant (Olimex ESP32-P4-DevKit)</text>
            <text x="10.16" y="432.48" size="1.778" layer="97">J10 is the ECU side: 1 +12V switched, 2 GND, 3 CAN H, 4 CAN L. J11 is the gauge chain: 1 +5V, 2 GND, 3 CAN H, 4 CAN L.</text>
            <text x="10.16" y="429.48" size="1.778" layer="97">U4 is the Olimex ESP32-P4-DevKit (rev C), pins down, USB-C end at EXT pin 1. It takes its 3.3 V rail to the carrier on EXT1-1.</text>
            <text x="10.16" y="426.48" size="1.778" layer="97">J13 feeds the DevKit: wire J13-1 to POE_PWR1 pin 4 (+5VP) and J13-2 to pin 3 (GND). Leave POE_PWR1 pins 1 and 2 open.</text>
            <text x="10.16" y="423.48" size="1.778" layer="97">Do not feed the DevKit through EXT2-1 (+5V): with USB plugged in, that pin back-feeds the PC.</text>
            <text x="10.16" y="420.48" size="1.778" layer="97">U6 is the Wi-Fi radio (ESP-Hosted over SDIO, the pins Espressif's own P4 board uses). Antenna at a board edge, no copper under it.</text>
            <text x="10.16" y="417.48" size="1.778" layer="97">U5 is the clock. BT1 is a CR2032; fit JP2 only with a rechargeable ML2032, never with a CR2032.</text>
            <text x="10.16" y="414.48" size="1.778" layer="97">J12 is the optional GPS header: 1 VCC (JP3 picks 3.3 V or 5 V), 2 GND, 3 TXD from the module, 4 RXD to it, 5 PPS.</text>
            <text x="10.16" y="411.48" size="1.778" layer="97">The DevKit already has 2.2 k pull-ups on SDA and SCL (GPIO7, GPIO8). Footprints are generic or placeholders.</text>
          </plain>
          <instances>
            <instance part="J10" gate="G$1" x="35.56" y="381" />
            <instance part="D1" gate="G$1" x="86.36" y="381" />
            <instance part="D2" gate="G$1" x="137.16" y="381" />
            <instance part="C1" gate="G$1" x="187.96" y="381" />
            <instance part="C2" gate="G$1" x="238.76" y="381" />
            <instance part="U1" gate="G$1" x="45.72" y="345.44" />
            <instance part="C3" gate="G$1" x="116.84" y="350.52" />
            <instance part="C4" gate="G$1" x="167.64" y="350.52" />
            <instance part="C5" gate="G$1" x="218.44" y="350.52" />
            <instance part="J13" gate="G$1" x="121.92" y="330.2" />
            <instance part="F2" gate="G$1" x="177.8" y="332.74" />
            <instance part="J11" gate="G$1" x="279.4" y="345.44" />
            <instance part="U4" gate="G$1" x="76.2" y="254" />
            <instance part="U2" gate="G$1" x="182.88" y="284.48" />
            <instance part="U3" gate="G$1" x="182.88" y="238.76" />
            <instance part="D4" gate="G$1" x="254" y="294.64" />
            <instance part="R3" gate="G$1" x="254" y="276.86" />
            <instance part="JP1" gate="G$1" x="304.8" y="276.86" />
            <instance part="D5" gate="G$1" x="254" y="254" />
            <instance part="R4" gate="G$1" x="254" y="236.22" />
            <instance part="R5" gate="G$1" x="304.8" y="236.22" />
            <instance part="R6" gate="G$1" x="254" y="223.52" />
            <instance part="R7" gate="G$1" x="304.8" y="223.52" />
            <instance part="C9" gate="G$1" x="254" y="210.82" />
            <instance part="J12" gate="G$1" x="386.08" y="289.56" />
            <instance part="JP3" gate="G$1" x="386.08" y="256.54" />
            <instance part="R22" gate="G$1" x="381" y="233.68" />
            <instance part="R23" gate="G$1" x="381" y="220.98" />
            <instance part="C16" gate="G$1" x="381" y="208.28" />
            <instance part="U6" gate="G$1" x="76.2" y="127" />
            <instance part="R10" gate="G$1" x="157.48" y="157.48" />
            <instance part="C13" gate="G$1" x="213.36" y="157.48" />
            <instance part="C11" gate="G$1" x="269.24" y="157.48" />
            <instance part="C12" gate="G$1" x="325.12" y="157.48" />
            <instance part="R11" gate="G$1" x="157.48" y="142.24" />
            <instance part="R12" gate="G$1" x="213.36" y="142.24" />
            <instance part="R13" gate="G$1" x="269.24" y="142.24" />
            <instance part="R14" gate="G$1" x="325.12" y="142.24" />
            <instance part="R15" gate="G$1" x="157.48" y="127" />
            <instance part="R16" gate="G$1" x="213.36" y="127" />
            <instance part="R17" gate="G$1" x="269.24" y="127" />
            <instance part="TP6" gate="G$1" x="162.56" y="111.76" />
            <instance part="TP7" gate="G$1" x="218.44" y="111.76" />
            <instance part="TP8" gate="G$1" x="274.32" y="111.76" />
            <instance part="TP9" gate="G$1" x="330.2" y="111.76" />
            <instance part="C6" gate="G$1" x="157.48" y="96.52" />
            <instance part="C7" gate="G$1" x="213.36" y="96.52" />
            <instance part="C8" gate="G$1" x="269.24" y="96.52" />
            <instance part="C10" gate="G$1" x="325.12" y="96.52" />
            <instance part="R1" gate="G$1" x="157.48" y="83.82" />
            <instance part="R2" gate="G$1" x="213.36" y="83.82" />
            <instance part="R8" gate="G$1" x="269.24" y="83.82" />
            <instance part="R9" gate="G$1" x="325.12" y="83.82" />
            <instance part="U5" gate="G$1" x="76.2" y="40.64" />
            <instance part="C14" gate="G$1" x="157.48" y="66.04" />
            <instance part="R20" gate="G$1" x="213.36" y="66.04" />
            <instance part="BT1" gate="G$1" x="157.48" y="48.26" />
            <instance part="C15" gate="G$1" x="213.36" y="48.26" />
            <instance part="D6" gate="G$1" x="157.48" y="30.48" />
            <instance part="R21" gate="G$1" x="213.36" y="30.48" />
            <instance part="JP2" gate="G$1" x="269.24" y="30.48" />
            <instance part="TP1" gate="G$1" x="386.08" y="381" />
            <instance part="TP2" gate="G$1" x="386.08" y="370.84" />
            <instance part="TP3" gate="G$1" x="386.08" y="360.68" />
            <instance part="TP4" gate="G$1" x="386.08" y="350.52" />
            <instance part="TP5" gate="G$1" x="386.08" y="340.36" />
          </instances>
          <busses />
          <nets>
            <net name="+12V_IN" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="1" />
                <wire x1="25.4" y1="388.62" x2="22.86" y2="388.62" width="0.1524" layer="91" />
                <label x="22.86" y="388.62" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D1" gate="G$1" pin="1" />
                <wire x1="81.28" y1="381" x2="78.74" y2="381" width="0.1524" layer="91" />
                <label x="78.74" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D2" gate="G$1" pin="A" />
                <wire x1="132.08" y1="381" x2="129.54" y2="381" width="0.1524" layer="91" />
                <label x="129.54" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R8" gate="G$1" pin="1" />
                <wire x1="264.16" y1="83.82" x2="261.62" y2="83.82" width="0.1524" layer="91" />
                <label x="261.62" y="83.82" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+12V_PROT" class="0">
              <segment>
                <pinref part="D2" gate="G$1" pin="K" />
                <wire x1="142.24" y1="381" x2="144.78" y2="381" width="0.1524" layer="91" />
                <label x="144.78" y="381" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="+" />
                <wire x1="182.88" y1="381" x2="180.34" y2="381" width="0.1524" layer="91" />
                <label x="180.34" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C2" gate="G$1" pin="1" />
                <wire x1="233.68" y1="381" x2="231.14" y2="381" width="0.1524" layer="91" />
                <label x="231.14" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="VIN" />
                <wire x1="30.48" y1="350.52" x2="27.94" y2="350.52" width="0.1524" layer="91" />
                <label x="27.94" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V" class="0">
              <segment>
                <pinref part="U1" gate="G$1" pin="VOUT" />
                <wire x1="60.96" y1="350.52" x2="63.5" y2="350.52" width="0.1524" layer="91" />
                <label x="63.5" y="350.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C3" gate="G$1" pin="+" />
                <wire x1="111.76" y1="350.52" x2="109.22" y2="350.52" width="0.1524" layer="91" />
                <label x="109.22" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="1" />
                <wire x1="162.56" y1="350.52" x2="160.02" y2="350.52" width="0.1524" layer="91" />
                <label x="160.02" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="1" />
                <wire x1="213.36" y1="350.52" x2="210.82" y2="350.52" width="0.1524" layer="91" />
                <label x="210.82" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="F2" gate="G$1" pin="1" />
                <wire x1="172.72" y1="332.74" x2="170.18" y2="332.74" width="0.1524" layer="91" />
                <label x="170.18" y="332.74" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="+5V" />
                <wire x1="109.22" y1="332.74" x2="106.68" y2="332.74" width="0.1524" layer="91" />
                <label x="106.68" y="332.74" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VCC" />
                <wire x1="167.64" y1="276.86" x2="165.1" y2="276.86" width="0.1524" layer="91" />
                <label x="165.1" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C6" gate="G$1" pin="1" />
                <wire x1="152.4" y1="96.52" x2="149.86" y2="96.52" width="0.1524" layer="91" />
                <label x="149.86" y="96.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP3" gate="G$1" pin="3" />
                <wire x1="375.92" y1="251.46" x2="373.38" y2="251.46" width="0.1524" layer="91" />
                <label x="373.38" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP2" gate="G$1" pin="TP" />
                <wire x1="383.54" y1="370.84" x2="381" y2="370.84" width="0.1524" layer="91" />
                <label x="381" y="370.84" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V_CHAIN" class="0">
              <segment>
                <pinref part="F2" gate="G$1" pin="2" />
                <wire x1="182.88" y1="332.74" x2="185.42" y2="332.74" width="0.1524" layer="91" />
                <label x="185.42" y="332.74" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="1" />
                <wire x1="269.24" y1="353.06" x2="266.7" y2="353.06" width="0.1524" layer="91" />
                <label x="266.7" y="353.06" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+3V3" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="3V3" />
                <wire x1="55.88" y1="205.74" x2="53.34" y2="205.74" width="0.1524" layer="91" />
                <label x="53.34" y="205.74" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VIO" />
                <wire x1="167.64" y1="281.94" x2="165.1" y2="281.94" width="0.1524" layer="91" />
                <label x="165.1" y="281.94" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="1" />
                <wire x1="208.28" y1="96.52" x2="205.74" y2="96.52" width="0.1524" layer="91" />
                <label x="205.74" y="96.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="VCC" />
                <wire x1="167.64" y1="231.14" x2="165.1" y2="231.14" width="0.1524" layer="91" />
                <label x="165.1" y="231.14" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="1" />
                <wire x1="264.16" y1="96.52" x2="261.62" y2="96.52" width="0.1524" layer="91" />
                <label x="261.62" y="96.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="1" />
                <wire x1="208.28" y1="83.82" x2="205.74" y2="83.82" width="0.1524" layer="91" />
                <label x="205.74" y="83.82" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP3" gate="G$1" pin="TP" />
                <wire x1="383.54" y1="360.68" x2="381" y2="360.68" width="0.1524" layer="91" />
                <label x="381" y="360.68" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="3V3" />
                <wire x1="58.42" y1="160.02" x2="55.88" y2="160.02" width="0.1524" layer="91" />
                <label x="55.88" y="160.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C11" gate="G$1" pin="1" />
                <wire x1="264.16" y1="157.48" x2="261.62" y2="157.48" width="0.1524" layer="91" />
                <label x="261.62" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C12" gate="G$1" pin="1" />
                <wire x1="320.04" y1="157.48" x2="317.5" y2="157.48" width="0.1524" layer="91" />
                <label x="317.5" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R10" gate="G$1" pin="1" />
                <wire x1="152.4" y1="157.48" x2="149.86" y2="157.48" width="0.1524" layer="91" />
                <label x="149.86" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R11" gate="G$1" pin="1" />
                <wire x1="152.4" y1="142.24" x2="149.86" y2="142.24" width="0.1524" layer="91" />
                <label x="149.86" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R12" gate="G$1" pin="1" />
                <wire x1="208.28" y1="142.24" x2="205.74" y2="142.24" width="0.1524" layer="91" />
                <label x="205.74" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R13" gate="G$1" pin="1" />
                <wire x1="264.16" y1="142.24" x2="261.62" y2="142.24" width="0.1524" layer="91" />
                <label x="261.62" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R14" gate="G$1" pin="1" />
                <wire x1="320.04" y1="142.24" x2="317.5" y2="142.24" width="0.1524" layer="91" />
                <label x="317.5" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R15" gate="G$1" pin="1" />
                <wire x1="152.4" y1="127" x2="149.86" y2="127" width="0.1524" layer="91" />
                <label x="149.86" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R16" gate="G$1" pin="1" />
                <wire x1="208.28" y1="127" x2="205.74" y2="127" width="0.1524" layer="91" />
                <label x="205.74" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R17" gate="G$1" pin="1" />
                <wire x1="264.16" y1="127" x2="261.62" y2="127" width="0.1524" layer="91" />
                <label x="261.62" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="VCC" />
                <wire x1="58.42" y1="58.42" x2="55.88" y2="58.42" width="0.1524" layer="91" />
                <label x="55.88" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C14" gate="G$1" pin="1" />
                <wire x1="152.4" y1="66.04" x2="149.86" y2="66.04" width="0.1524" layer="91" />
                <label x="149.86" y="66.04" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R20" gate="G$1" pin="1" />
                <wire x1="208.28" y1="66.04" x2="205.74" y2="66.04" width="0.1524" layer="91" />
                <label x="205.74" y="66.04" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D6" gate="G$1" pin="A" />
                <wire x1="152.4" y1="30.48" x2="149.86" y2="30.48" width="0.1524" layer="91" />
                <label x="149.86" y="30.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP3" gate="G$1" pin="1" />
                <wire x1="375.92" y1="261.62" x2="373.38" y2="261.62" width="0.1524" layer="91" />
                <label x="373.38" y="261.62" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GND" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="2" />
                <wire x1="25.4" y1="383.54" x2="22.86" y2="383.54" width="0.1524" layer="91" />
                <label x="22.86" y="383.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D1" gate="G$1" pin="2" />
                <wire x1="91.44" y1="381" x2="93.98" y2="381" width="0.1524" layer="91" />
                <label x="93.98" y="381" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="-" />
                <wire x1="193.04" y1="381" x2="195.58" y2="381" width="0.1524" layer="91" />
                <label x="195.58" y="381" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C2" gate="G$1" pin="2" />
                <wire x1="243.84" y1="381" x2="246.38" y2="381" width="0.1524" layer="91" />
                <label x="246.38" y="381" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="GND@1" />
                <wire x1="30.48" y1="340.36" x2="27.94" y2="340.36" width="0.1524" layer="91" />
                <label x="27.94" y="340.36" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="GND@2" />
                <wire x1="60.96" y1="345.44" x2="63.5" y2="345.44" width="0.1524" layer="91" />
                <label x="63.5" y="345.44" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C3" gate="G$1" pin="-" />
                <wire x1="121.92" y1="350.52" x2="124.46" y2="350.52" width="0.1524" layer="91" />
                <label x="124.46" y="350.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="2" />
                <wire x1="172.72" y1="350.52" x2="175.26" y2="350.52" width="0.1524" layer="91" />
                <label x="175.26" y="350.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="2" />
                <wire x1="223.52" y1="350.52" x2="226.06" y2="350.52" width="0.1524" layer="91" />
                <label x="226.06" y="350.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="2" />
                <wire x1="269.24" y1="347.98" x2="266.7" y2="347.98" width="0.1524" layer="91" />
                <label x="266.7" y="347.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="GND" />
                <wire x1="109.22" y1="327.66" x2="106.68" y2="327.66" width="0.1524" layer="91" />
                <label x="106.68" y="327.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@1" />
                <wire x1="55.88" y1="210.82" x2="53.34" y2="210.82" width="0.1524" layer="91" />
                <label x="53.34" y="210.82" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@2" />
                <wire x1="96.52" y1="210.82" x2="99.06" y2="210.82" width="0.1524" layer="91" />
                <label x="99.06" y="210.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@3" />
                <wire x1="96.52" y1="276.86" x2="99.06" y2="276.86" width="0.1524" layer="91" />
                <label x="99.06" y="276.86" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@4" />
                <wire x1="96.52" y1="292.1" x2="99.06" y2="292.1" width="0.1524" layer="91" />
                <label x="99.06" y="292.1" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="GND" />
                <wire x1="167.64" y1="271.78" x2="165.1" y2="271.78" width="0.1524" layer="91" />
                <label x="165.1" y="271.78" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="GND" />
                <wire x1="167.64" y1="226.06" x2="165.1" y2="226.06" width="0.1524" layer="91" />
                <label x="165.1" y="226.06" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="RS" />
                <wire x1="167.64" y1="241.3" x2="165.1" y2="241.3" width="0.1524" layer="91" />
                <label x="165.1" y="241.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="GND" />
                <wire x1="264.16" y1="297.18" x2="266.7" y2="297.18" width="0.1524" layer="91" />
                <label x="266.7" y="297.18" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="GND" />
                <wire x1="264.16" y1="256.54" x2="266.7" y2="256.54" width="0.1524" layer="91" />
                <label x="266.7" y="256.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C9" gate="G$1" pin="2" />
                <wire x1="259.08" y1="210.82" x2="261.62" y2="210.82" width="0.1524" layer="91" />
                <label x="261.62" y="210.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C6" gate="G$1" pin="2" />
                <wire x1="162.56" y1="96.52" x2="165.1" y2="96.52" width="0.1524" layer="91" />
                <label x="165.1" y="96.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="2" />
                <wire x1="218.44" y1="96.52" x2="220.98" y2="96.52" width="0.1524" layer="91" />
                <label x="220.98" y="96.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="2" />
                <wire x1="274.32" y1="96.52" x2="276.86" y2="96.52" width="0.1524" layer="91" />
                <label x="276.86" y="96.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="2" />
                <wire x1="330.2" y1="96.52" x2="332.74" y2="96.52" width="0.1524" layer="91" />
                <label x="332.74" y="96.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="2" />
                <wire x1="162.56" y1="83.82" x2="165.1" y2="83.82" width="0.1524" layer="91" />
                <label x="165.1" y="83.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="2" />
                <wire x1="330.2" y1="83.82" x2="332.74" y2="83.82" width="0.1524" layer="91" />
                <label x="332.74" y="83.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP1" gate="G$1" pin="TP" />
                <wire x1="383.54" y1="381" x2="381" y2="381" width="0.1524" layer="91" />
                <label x="381" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@1" />
                <wire x1="93.98" y1="149.86" x2="96.52" y2="149.86" width="0.1524" layer="91" />
                <label x="96.52" y="149.86" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@2" />
                <wire x1="93.98" y1="147.32" x2="96.52" y2="147.32" width="0.1524" layer="91" />
                <label x="96.52" y="147.32" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@11" />
                <wire x1="93.98" y1="144.78" x2="96.52" y2="144.78" width="0.1524" layer="91" />
                <label x="96.52" y="144.78" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@14" />
                <wire x1="93.98" y1="142.24" x2="96.52" y2="142.24" width="0.1524" layer="91" />
                <label x="96.52" y="142.24" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@36" />
                <wire x1="93.98" y1="139.7" x2="96.52" y2="139.7" width="0.1524" layer="91" />
                <label x="96.52" y="139.7" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@37" />
                <wire x1="93.98" y1="137.16" x2="96.52" y2="137.16" width="0.1524" layer="91" />
                <label x="96.52" y="137.16" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@38" />
                <wire x1="93.98" y1="134.62" x2="96.52" y2="134.62" width="0.1524" layer="91" />
                <label x="96.52" y="134.62" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@39" />
                <wire x1="93.98" y1="132.08" x2="96.52" y2="132.08" width="0.1524" layer="91" />
                <label x="96.52" y="132.08" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@40" />
                <wire x1="93.98" y1="129.54" x2="96.52" y2="129.54" width="0.1524" layer="91" />
                <label x="96.52" y="129.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@41" />
                <wire x1="93.98" y1="127" x2="96.52" y2="127" width="0.1524" layer="91" />
                <label x="96.52" y="127" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@42" />
                <wire x1="93.98" y1="124.46" x2="96.52" y2="124.46" width="0.1524" layer="91" />
                <label x="96.52" y="124.46" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@43" />
                <wire x1="93.98" y1="121.92" x2="96.52" y2="121.92" width="0.1524" layer="91" />
                <label x="96.52" y="121.92" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@44" />
                <wire x1="93.98" y1="119.38" x2="96.52" y2="119.38" width="0.1524" layer="91" />
                <label x="96.52" y="119.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@45" />
                <wire x1="93.98" y1="116.84" x2="96.52" y2="116.84" width="0.1524" layer="91" />
                <label x="96.52" y="116.84" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@46" />
                <wire x1="93.98" y1="114.3" x2="96.52" y2="114.3" width="0.1524" layer="91" />
                <label x="96.52" y="114.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@47" />
                <wire x1="93.98" y1="111.76" x2="96.52" y2="111.76" width="0.1524" layer="91" />
                <label x="96.52" y="111.76" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@48" />
                <wire x1="93.98" y1="109.22" x2="96.52" y2="109.22" width="0.1524" layer="91" />
                <label x="96.52" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@49" />
                <wire x1="93.98" y1="106.68" x2="96.52" y2="106.68" width="0.1524" layer="91" />
                <label x="96.52" y="106.68" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@50" />
                <wire x1="93.98" y1="104.14" x2="96.52" y2="104.14" width="0.1524" layer="91" />
                <label x="96.52" y="104.14" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@51" />
                <wire x1="93.98" y1="101.6" x2="96.52" y2="101.6" width="0.1524" layer="91" />
                <label x="96.52" y="101.6" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@52" />
                <wire x1="93.98" y1="99.06" x2="96.52" y2="99.06" width="0.1524" layer="91" />
                <label x="96.52" y="99.06" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND@53" />
                <wire x1="93.98" y1="96.52" x2="96.52" y2="96.52" width="0.1524" layer="91" />
                <label x="96.52" y="96.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C11" gate="G$1" pin="2" />
                <wire x1="274.32" y1="157.48" x2="276.86" y2="157.48" width="0.1524" layer="91" />
                <label x="276.86" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C12" gate="G$1" pin="2" />
                <wire x1="330.2" y1="157.48" x2="332.74" y2="157.48" width="0.1524" layer="91" />
                <label x="332.74" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C13" gate="G$1" pin="2" />
                <wire x1="218.44" y1="157.48" x2="220.98" y2="157.48" width="0.1524" layer="91" />
                <label x="220.98" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="GND" />
                <wire x1="58.42" y1="22.86" x2="55.88" y2="22.86" width="0.1524" layer="91" />
                <label x="55.88" y="22.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@5" />
                <wire x1="93.98" y1="58.42" x2="96.52" y2="58.42" width="0.1524" layer="91" />
                <label x="96.52" y="58.42" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@6" />
                <wire x1="93.98" y1="53.34" x2="96.52" y2="53.34" width="0.1524" layer="91" />
                <label x="96.52" y="53.34" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@7" />
                <wire x1="93.98" y1="48.26" x2="96.52" y2="48.26" width="0.1524" layer="91" />
                <label x="96.52" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@8" />
                <wire x1="93.98" y1="43.18" x2="96.52" y2="43.18" width="0.1524" layer="91" />
                <label x="96.52" y="43.18" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@9" />
                <wire x1="93.98" y1="38.1" x2="96.52" y2="38.1" width="0.1524" layer="91" />
                <label x="96.52" y="38.1" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@10" />
                <wire x1="93.98" y1="33.02" x2="96.52" y2="33.02" width="0.1524" layer="91" />
                <label x="96.52" y="33.02" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@11" />
                <wire x1="93.98" y1="27.94" x2="96.52" y2="27.94" width="0.1524" layer="91" />
                <label x="96.52" y="27.94" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@12" />
                <wire x1="93.98" y1="22.86" x2="96.52" y2="22.86" width="0.1524" layer="91" />
                <label x="96.52" y="22.86" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C14" gate="G$1" pin="2" />
                <wire x1="162.56" y1="66.04" x2="165.1" y2="66.04" width="0.1524" layer="91" />
                <label x="165.1" y="66.04" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="BT1" gate="G$1" pin="-" />
                <wire x1="162.56" y1="48.26" x2="165.1" y2="48.26" width="0.1524" layer="91" />
                <label x="165.1" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C15" gate="G$1" pin="2" />
                <wire x1="218.44" y1="48.26" x2="220.98" y2="48.26" width="0.1524" layer="91" />
                <label x="220.98" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J12" gate="G$1" pin="GND" />
                <wire x1="373.38" y1="294.64" x2="370.84" y2="294.64" width="0.1524" layer="91" />
                <label x="370.84" y="294.64" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C16" gate="G$1" pin="2" />
                <wire x1="386.08" y1="208.28" x2="388.62" y2="208.28" width="0.1524" layer="91" />
                <label x="388.62" y="208.28" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="ECU_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO21" />
                <wire x1="96.52" y1="261.62" x2="99.06" y2="261.62" width="0.1524" layer="91" />
                <label x="99.06" y="261.62" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="TXD" />
                <wire x1="167.64" y1="297.18" x2="165.1" y2="297.18" width="0.1524" layer="91" />
                <label x="165.1" y="297.18" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO22" />
                <wire x1="96.52" y1="256.54" x2="99.06" y2="256.54" width="0.1524" layer="91" />
                <label x="99.06" y="256.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="RXD" />
                <wire x1="167.64" y1="292.1" x2="165.1" y2="292.1" width="0.1524" layer="91" />
                <label x="165.1" y="292.1" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_SILENT" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO23" />
                <wire x1="96.52" y1="251.46" x2="99.06" y2="251.46" width="0.1524" layer="91" />
                <label x="99.06" y="251.46" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="S" />
                <wire x1="167.64" y1="287.02" x2="165.1" y2="287.02" width="0.1524" layer="91" />
                <label x="165.1" y="287.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="1" />
                <wire x1="152.4" y1="83.82" x2="149.86" y2="83.82" width="0.1524" layer="91" />
                <label x="149.86" y="83.82" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GAUGE_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO32" />
                <wire x1="96.52" y1="246.38" x2="99.06" y2="246.38" width="0.1524" layer="91" />
                <label x="99.06" y="246.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="D" />
                <wire x1="167.64" y1="251.46" x2="165.1" y2="251.46" width="0.1524" layer="91" />
                <label x="165.1" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="2" />
                <wire x1="218.44" y1="83.82" x2="220.98" y2="83.82" width="0.1524" layer="91" />
                <label x="220.98" y="83.82" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="GAUGE_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO33" />
                <wire x1="96.52" y1="241.3" x2="99.06" y2="241.3" width="0.1524" layer="91" />
                <label x="99.06" y="241.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="R" />
                <wire x1="167.64" y1="246.38" x2="165.1" y2="246.38" width="0.1524" layer="91" />
                <label x="165.1" y="246.38" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="VBAT_SENSE" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO20" />
                <wire x1="96.52" y1="266.7" x2="99.06" y2="266.7" width="0.1524" layer="91" />
                <label x="99.06" y="266.7" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R8" gate="G$1" pin="2" />
                <wire x1="274.32" y1="83.82" x2="276.86" y2="83.82" width="0.1524" layer="91" />
                <label x="276.86" y="83.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="1" />
                <wire x1="320.04" y1="83.82" x2="317.5" y2="83.82" width="0.1524" layer="91" />
                <label x="317.5" y="83.82" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="1" />
                <wire x1="320.04" y1="96.52" x2="317.5" y2="96.52" width="0.1524" layer="91" />
                <label x="317.5" y="96.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP4" gate="G$1" pin="TP" />
                <wire x1="383.54" y1="350.52" x2="381" y2="350.52" width="0.1524" layer="91" />
                <label x="381" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RST" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="EN" />
                <wire x1="96.52" y1="271.78" x2="99.06" y2="271.78" width="0.1524" layer="91" />
                <label x="99.06" y="271.78" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP5" gate="G$1" pin="TP" />
                <wire x1="383.54" y1="340.36" x2="381" y2="340.36" width="0.1524" layer="91" />
                <label x="381" y="340.36" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_CANH" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="3" />
                <wire x1="25.4" y1="378.46" x2="22.86" y2="378.46" width="0.1524" layer="91" />
                <label x="22.86" y="378.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="CANH" />
                <wire x1="198.12" y1="297.18" x2="200.66" y2="297.18" width="0.1524" layer="91" />
                <label x="200.66" y="297.18" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="IO1" />
                <wire x1="243.84" y1="297.18" x2="241.3" y2="297.18" width="0.1524" layer="91" />
                <label x="241.3" y="297.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R3" gate="G$1" pin="1" />
                <wire x1="248.92" y1="276.86" x2="246.38" y2="276.86" width="0.1524" layer="91" />
                <label x="246.38" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_CANL" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="4" />
                <wire x1="25.4" y1="373.38" x2="22.86" y2="373.38" width="0.1524" layer="91" />
                <label x="22.86" y="373.38" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="CANL" />
                <wire x1="198.12" y1="292.1" x2="200.66" y2="292.1" width="0.1524" layer="91" />
                <label x="200.66" y="292.1" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="IO2" />
                <wire x1="243.84" y1="292.1" x2="241.3" y2="292.1" width="0.1524" layer="91" />
                <label x="241.3" y="292.1" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP1" gate="G$1" pin="2" />
                <wire x1="309.88" y1="276.86" x2="312.42" y2="276.86" width="0.1524" layer="91" />
                <label x="312.42" y="276.86" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="ECU_TERM" class="0">
              <segment>
                <pinref part="R3" gate="G$1" pin="2" />
                <wire x1="259.08" y1="276.86" x2="261.62" y2="276.86" width="0.1524" layer="91" />
                <label x="261.62" y="276.86" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP1" gate="G$1" pin="1" />
                <wire x1="299.72" y1="276.86" x2="297.18" y2="276.86" width="0.1524" layer="91" />
                <label x="297.18" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANH" class="0">
              <segment>
                <pinref part="J11" gate="G$1" pin="3" />
                <wire x1="269.24" y1="342.9" x2="266.7" y2="342.9" width="0.1524" layer="91" />
                <label x="266.7" y="342.9" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="CANH" />
                <wire x1="198.12" y1="251.46" x2="200.66" y2="251.46" width="0.1524" layer="91" />
                <label x="200.66" y="251.46" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="IO1" />
                <wire x1="243.84" y1="256.54" x2="241.3" y2="256.54" width="0.1524" layer="91" />
                <label x="241.3" y="256.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R4" gate="G$1" pin="1" />
                <wire x1="248.92" y1="236.22" x2="246.38" y2="236.22" width="0.1524" layer="91" />
                <label x="246.38" y="236.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R5" gate="G$1" pin="1" />
                <wire x1="299.72" y1="236.22" x2="297.18" y2="236.22" width="0.1524" layer="91" />
                <label x="297.18" y="236.22" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANL" class="0">
              <segment>
                <pinref part="J11" gate="G$1" pin="4" />
                <wire x1="269.24" y1="337.82" x2="266.7" y2="337.82" width="0.1524" layer="91" />
                <label x="266.7" y="337.82" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="CANL" />
                <wire x1="198.12" y1="246.38" x2="200.66" y2="246.38" width="0.1524" layer="91" />
                <label x="200.66" y="246.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="IO2" />
                <wire x1="243.84" y1="251.46" x2="241.3" y2="251.46" width="0.1524" layer="91" />
                <label x="241.3" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R6" gate="G$1" pin="2" />
                <wire x1="259.08" y1="223.52" x2="261.62" y2="223.52" width="0.1524" layer="91" />
                <label x="261.62" y="223.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R7" gate="G$1" pin="2" />
                <wire x1="309.88" y1="223.52" x2="312.42" y2="223.52" width="0.1524" layer="91" />
                <label x="312.42" y="223.52" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="TERM_MID" class="0">
              <segment>
                <pinref part="R4" gate="G$1" pin="2" />
                <wire x1="259.08" y1="236.22" x2="261.62" y2="236.22" width="0.1524" layer="91" />
                <label x="261.62" y="236.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R5" gate="G$1" pin="2" />
                <wire x1="309.88" y1="236.22" x2="312.42" y2="236.22" width="0.1524" layer="91" />
                <label x="312.42" y="236.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R6" gate="G$1" pin="1" />
                <wire x1="248.92" y1="223.52" x2="246.38" y2="223.52" width="0.1524" layer="91" />
                <label x="246.38" y="223.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R7" gate="G$1" pin="1" />
                <wire x1="299.72" y1="223.52" x2="297.18" y2="223.52" width="0.1524" layer="91" />
                <label x="297.18" y="223.52" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C9" gate="G$1" pin="1" />
                <wire x1="248.92" y1="210.82" x2="246.38" y2="210.82" width="0.1524" layer="91" />
                <label x="246.38" y="210.82" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SDIO_CLK" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO18" />
                <wire x1="55.88" y1="297.18" x2="53.34" y2="297.18" width="0.1524" layer="91" />
                <label x="53.34" y="297.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO19" />
                <wire x1="58.42" y1="139.7" x2="55.88" y2="139.7" width="0.1524" layer="91" />
                <label x="55.88" y="139.7" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SDIO_CMD" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO19" />
                <wire x1="55.88" y1="302.26" x2="53.34" y2="302.26" width="0.1524" layer="91" />
                <label x="53.34" y="302.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO18" />
                <wire x1="58.42" y1="142.24" x2="55.88" y2="142.24" width="0.1524" layer="91" />
                <label x="55.88" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R11" gate="G$1" pin="2" />
                <wire x1="162.56" y1="142.24" x2="165.1" y2="142.24" width="0.1524" layer="91" />
                <label x="165.1" y="142.24" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="SDIO_D0" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO14" />
                <wire x1="55.88" y1="276.86" x2="53.34" y2="276.86" width="0.1524" layer="91" />
                <label x="53.34" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO20" />
                <wire x1="58.42" y1="137.16" x2="55.88" y2="137.16" width="0.1524" layer="91" />
                <label x="55.88" y="137.16" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R12" gate="G$1" pin="2" />
                <wire x1="218.44" y1="142.24" x2="220.98" y2="142.24" width="0.1524" layer="91" />
                <label x="220.98" y="142.24" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="SDIO_D1" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO15" />
                <wire x1="55.88" y1="281.94" x2="53.34" y2="281.94" width="0.1524" layer="91" />
                <label x="53.34" y="281.94" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO21" />
                <wire x1="58.42" y1="134.62" x2="55.88" y2="134.62" width="0.1524" layer="91" />
                <label x="55.88" y="134.62" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R13" gate="G$1" pin="2" />
                <wire x1="274.32" y1="142.24" x2="276.86" y2="142.24" width="0.1524" layer="91" />
                <label x="276.86" y="142.24" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="SDIO_D2" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO16" />
                <wire x1="55.88" y1="287.02" x2="53.34" y2="287.02" width="0.1524" layer="91" />
                <label x="53.34" y="287.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO22" />
                <wire x1="58.42" y1="132.08" x2="55.88" y2="132.08" width="0.1524" layer="91" />
                <label x="55.88" y="132.08" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R14" gate="G$1" pin="2" />
                <wire x1="330.2" y1="142.24" x2="332.74" y2="142.24" width="0.1524" layer="91" />
                <label x="332.74" y="142.24" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="SDIO_D3" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO17" />
                <wire x1="55.88" y1="292.1" x2="53.34" y2="292.1" width="0.1524" layer="91" />
                <label x="53.34" y="292.1" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO23" />
                <wire x1="58.42" y1="129.54" x2="55.88" y2="129.54" width="0.1524" layer="91" />
                <label x="55.88" y="129.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R15" gate="G$1" pin="2" />
                <wire x1="162.56" y1="127" x2="165.1" y2="127" width="0.1524" layer="91" />
                <label x="165.1" y="127" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="C6_EN" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO54" />
                <wire x1="96.52" y1="215.9" x2="99.06" y2="215.9" width="0.1524" layer="91" />
                <label x="99.06" y="215.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="EN" />
                <wire x1="58.42" y1="157.48" x2="55.88" y2="157.48" width="0.1524" layer="91" />
                <label x="55.88" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R10" gate="G$1" pin="2" />
                <wire x1="162.56" y1="157.48" x2="165.1" y2="157.48" width="0.1524" layer="91" />
                <label x="165.1" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C13" gate="G$1" pin="1" />
                <wire x1="208.28" y1="157.48" x2="205.74" y2="157.48" width="0.1524" layer="91" />
                <label x="205.74" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP6" gate="G$1" pin="TP" />
                <wire x1="160.02" y1="111.76" x2="157.48" y2="111.76" width="0.1524" layer="91" />
                <label x="157.48" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="C6_WAKE" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO6" />
                <wire x1="55.88" y1="236.22" x2="53.34" y2="236.22" width="0.1524" layer="91" />
                <label x="53.34" y="236.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="IO2" />
                <wire x1="58.42" y1="144.78" x2="55.88" y2="144.78" width="0.1524" layer="91" />
                <label x="55.88" y="144.78" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="C6_TXD" class="0">
              <segment>
                <pinref part="U6" gate="G$1" pin="TXD0" />
                <wire x1="58.42" y1="147.32" x2="55.88" y2="147.32" width="0.1524" layer="91" />
                <label x="55.88" y="147.32" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO5" />
                <wire x1="55.88" y1="231.14" x2="53.34" y2="231.14" width="0.1524" layer="91" />
                <label x="53.34" y="231.14" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP7" gate="G$1" pin="TP" />
                <wire x1="215.9" y1="111.76" x2="213.36" y2="111.76" width="0.1524" layer="91" />
                <label x="213.36" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="C6_RXD" class="0">
              <segment>
                <pinref part="U6" gate="G$1" pin="RXD0" />
                <wire x1="58.42" y1="149.86" x2="55.88" y2="149.86" width="0.1524" layer="91" />
                <label x="55.88" y="149.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO4" />
                <wire x1="55.88" y1="226.06" x2="53.34" y2="226.06" width="0.1524" layer="91" />
                <label x="53.34" y="226.06" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP8" gate="G$1" pin="TP" />
                <wire x1="271.78" y1="111.76" x2="269.24" y2="111.76" width="0.1524" layer="91" />
                <label x="269.24" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="C6_BOOT" class="0">
              <segment>
                <pinref part="U6" gate="G$1" pin="IO9" />
                <wire x1="58.42" y1="152.4" x2="55.88" y2="152.4" width="0.1524" layer="91" />
                <label x="55.88" y="152.4" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO53" />
                <wire x1="96.52" y1="220.98" x2="99.06" y2="220.98" width="0.1524" layer="91" />
                <label x="99.06" y="220.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R16" gate="G$1" pin="2" />
                <wire x1="218.44" y1="127" x2="220.98" y2="127" width="0.1524" layer="91" />
                <label x="220.98" y="127" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP9" gate="G$1" pin="TP" />
                <wire x1="327.66" y1="111.76" x2="325.12" y2="111.76" width="0.1524" layer="91" />
                <label x="325.12" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="C6_IO8" class="0">
              <segment>
                <pinref part="U6" gate="G$1" pin="IO8" />
                <wire x1="58.42" y1="154.94" x2="55.88" y2="154.94" width="0.1524" layer="91" />
                <label x="55.88" y="154.94" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R17" gate="G$1" pin="2" />
                <wire x1="274.32" y1="127" x2="276.86" y2="127" width="0.1524" layer="91" />
                <label x="276.86" y="127" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="I2C_SDA" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO7_SDA" />
                <wire x1="55.88" y1="241.3" x2="53.34" y2="241.3" width="0.1524" layer="91" />
                <label x="53.34" y="241.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="SDA" />
                <wire x1="58.42" y1="48.26" x2="55.88" y2="48.26" width="0.1524" layer="91" />
                <label x="55.88" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="I2C_SCL" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO8_SCL" />
                <wire x1="55.88" y1="246.38" x2="53.34" y2="246.38" width="0.1524" layer="91" />
                <label x="53.34" y="246.38" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="SCL" />
                <wire x1="58.42" y1="43.18" x2="55.88" y2="43.18" width="0.1524" layer="91" />
                <label x="55.88" y="43.18" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RTC_INT" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO9" />
                <wire x1="55.88" y1="251.46" x2="53.34" y2="251.46" width="0.1524" layer="91" />
                <label x="53.34" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="INT_SQW" />
                <wire x1="58.42" y1="38.1" x2="55.88" y2="38.1" width="0.1524" layer="91" />
                <label x="55.88" y="38.1" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R20" gate="G$1" pin="2" />
                <wire x1="218.44" y1="66.04" x2="220.98" y2="66.04" width="0.1524" layer="91" />
                <label x="220.98" y="66.04" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="RTC_VBAT" class="0">
              <segment>
                <pinref part="U5" gate="G$1" pin="VBAT" />
                <wire x1="58.42" y1="53.34" x2="55.88" y2="53.34" width="0.1524" layer="91" />
                <label x="55.88" y="53.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="BT1" gate="G$1" pin="+" />
                <wire x1="152.4" y1="48.26" x2="149.86" y2="48.26" width="0.1524" layer="91" />
                <label x="149.86" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C15" gate="G$1" pin="1" />
                <wire x1="208.28" y1="48.26" x2="205.74" y2="48.26" width="0.1524" layer="91" />
                <label x="205.74" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP2" gate="G$1" pin="2" />
                <wire x1="274.32" y1="30.48" x2="276.86" y2="30.48" width="0.1524" layer="91" />
                <label x="276.86" y="30.48" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="RTC_CHG_A" class="0">
              <segment>
                <pinref part="D6" gate="G$1" pin="K" />
                <wire x1="162.56" y1="30.48" x2="165.1" y2="30.48" width="0.1524" layer="91" />
                <label x="165.1" y="30.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R21" gate="G$1" pin="1" />
                <wire x1="208.28" y1="30.48" x2="205.74" y2="30.48" width="0.1524" layer="91" />
                <label x="205.74" y="30.48" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RTC_CHG_B" class="0">
              <segment>
                <pinref part="R21" gate="G$1" pin="2" />
                <wire x1="218.44" y1="30.48" x2="220.98" y2="30.48" width="0.1524" layer="91" />
                <label x="220.98" y="30.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP2" gate="G$1" pin="1" />
                <wire x1="264.16" y1="30.48" x2="261.62" y2="30.48" width="0.1524" layer="91" />
                <label x="261.62" y="30.48" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_VCC" class="0">
              <segment>
                <pinref part="JP3" gate="G$1" pin="2" />
                <wire x1="375.92" y1="256.54" x2="373.38" y2="256.54" width="0.1524" layer="91" />
                <label x="373.38" y="256.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J12" gate="G$1" pin="VCC" />
                <wire x1="373.38" y1="299.72" x2="370.84" y2="299.72" width="0.1524" layer="91" />
                <label x="370.84" y="299.72" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C16" gate="G$1" pin="1" />
                <wire x1="375.92" y1="208.28" x2="373.38" y2="208.28" width="0.1524" layer="91" />
                <label x="373.38" y="208.28" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_TXD" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="TXD" />
                <wire x1="373.38" y1="289.56" x2="370.84" y2="289.56" width="0.1524" layer="91" />
                <label x="370.84" y="289.56" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R22" gate="G$1" pin="1" />
                <wire x1="375.92" y1="233.68" x2="373.38" y2="233.68" width="0.1524" layer="91" />
                <label x="373.38" y="233.68" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_RX" class="0">
              <segment>
                <pinref part="R22" gate="G$1" pin="2" />
                <wire x1="386.08" y1="233.68" x2="388.62" y2="233.68" width="0.1524" layer="91" />
                <label x="388.62" y="233.68" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO10" />
                <wire x1="55.88" y1="256.54" x2="53.34" y2="256.54" width="0.1524" layer="91" />
                <label x="53.34" y="256.54" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_RXD" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="RXD" />
                <wire x1="373.38" y1="284.48" x2="370.84" y2="284.48" width="0.1524" layer="91" />
                <label x="370.84" y="284.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO11" />
                <wire x1="55.88" y1="261.62" x2="53.34" y2="261.62" width="0.1524" layer="91" />
                <label x="53.34" y="261.62" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_PPS" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="PPS" />
                <wire x1="373.38" y1="279.4" x2="370.84" y2="279.4" width="0.1524" layer="91" />
                <label x="370.84" y="279.4" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R23" gate="G$1" pin="1" />
                <wire x1="375.92" y1="220.98" x2="373.38" y2="220.98" width="0.1524" layer="91" />
                <label x="373.38" y="220.98" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_PPS_IN" class="0">
              <segment>
                <pinref part="R23" gate="G$1" pin="2" />
                <wire x1="386.08" y1="220.98" x2="388.62" y2="220.98" width="0.1524" layer="91" />
                <label x="388.62" y="220.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO12" />
                <wire x1="55.88" y1="266.7" x2="53.34" y2="266.7" width="0.1524" layer="91" />
                <label x="53.34" y="266.7" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
          </nets>
        </sheet>
      </sheets>
    </schematic>
  </drawing>
</eagle>
