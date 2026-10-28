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
            <package name="HDR-1X6">
              <description>Pin header, 2.54 mm pitch, 1.0 mm drill.</description>
              <pad name="1" x="-6.35" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="-3.81" y="0" drill="1" diameter="1.8" />
              <pad name="3" x="-1.27" y="0" drill="1" diameter="1.8" />
              <pad name="4" x="1.27" y="0" drill="1" diameter="1.8" />
              <pad name="5" x="3.81" y="0" drill="1" diameter="1.8" />
              <pad name="6" x="6.35" y="0" drill="1" diameter="1.8" />
              <wire x1="-7.62" y1="1.27" x2="7.62" y2="1.27" width="0.127" layer="21" />
              <wire x1="7.62" y1="1.27" x2="7.62" y2="-1.27" width="0.127" layer="21" />
              <wire x1="7.62" y1="-1.27" x2="-7.62" y2="-1.27" width="0.127" layer="21" />
              <wire x1="-7.62" y1="-1.27" x2="-7.62" y2="1.27" width="0.127" layer="21" />
              <text x="-7.62" y="1.7" size="1.016" layer="25">&gt;NAME</text>
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
            <package name="MOLEX-43045-0612">
              <description>Molex Micro-Fit 3.0 vertical header, 2x3, drawn from the 2x2 pattern of SD-43045-005 widened by one 3.00 mm pitch. Check the peg spacing and body against the Molex drawing for the 6-circuit part before ordering boards.</description>
              <pad name="1" x="3" y="-1.5" drill="1.02" diameter="1.9" shape="square" />
              <pad name="2" x="0" y="-1.5" drill="1.02" diameter="1.9" />
              <pad name="3" x="-3" y="-1.5" drill="1.02" diameter="1.9" />
              <pad name="4" x="3" y="1.5" drill="1.02" diameter="1.9" />
              <pad name="5" x="0" y="1.5" drill="1.02" diameter="1.9" />
              <pad name="6" x="-3" y="1.5" drill="1.02" diameter="1.9" />
              <hole x="-6" y="2.44" drill="1.02" />
              <hole x="6" y="2.44" drill="1.02" />
              <wire x1="-6.325" y1="3.685" x2="6.325" y2="3.685" width="0.127" layer="21" />
              <wire x1="6.325" y1="3.685" x2="6.325" y2="-3.685" width="0.127" layer="21" />
              <wire x1="6.325" y1="-3.685" x2="-6.325" y2="-3.685" width="0.127" layer="21" />
              <wire x1="-6.325" y1="-3.685" x2="-6.325" y2="3.685" width="0.127" layer="21" />
              <text x="-6.325" y="4.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="MSOP8">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout. 0.65 mm pitch.</description>
              <smd name="1" x="-2" y="0.975" dx="1.45" dy="0.4" layer="1" />
              <smd name="8" x="2" y="0.975" dx="1.45" dy="0.4" layer="1" />
              <smd name="2" x="-2" y="0.325" dx="1.45" dy="0.4" layer="1" />
              <smd name="7" x="2" y="0.325" dx="1.45" dy="0.4" layer="1" />
              <smd name="3" x="-2" y="-0.325" dx="1.45" dy="0.4" layer="1" />
              <smd name="6" x="2" y="-0.325" dx="1.45" dy="0.4" layer="1" />
              <smd name="4" x="-2" y="-0.975" dx="1.45" dy="0.4" layer="1" />
              <smd name="5" x="2" y="-0.975" dx="1.45" dy="0.4" layer="1" />
              <wire x1="-1.5" y1="1.5" x2="1.5" y2="1.5" width="0.127" layer="21" />
              <wire x1="1.5" y1="1.5" x2="1.5" y2="-1.5" width="0.127" layer="21" />
              <wire x1="1.5" y1="-1.5" x2="-1.5" y2="-1.5" width="0.127" layer="21" />
              <wire x1="-1.5" y1="-1.5" x2="-1.5" y2="1.5" width="0.127" layer="21" />
              <text x="-1.5" y="1.9" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="AXIAL-10MM">
              <description>Through-hole resistor, 10.16 mm lead spacing, 1.0 mm drill.</description>
              <pad name="1" x="-5.08" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="5.08" y="0" drill="1" diameter="1.8" />
              <wire x1="-3.5" y1="1.3" x2="3.5" y2="1.3" width="0.127" layer="21" />
              <wire x1="3.5" y1="1.3" x2="3.5" y2="-1.3" width="0.127" layer="21" />
              <wire x1="3.5" y1="-1.3" x2="-3.5" y2="-1.3" width="0.127" layer="21" />
              <wire x1="-3.5" y1="-1.3" x2="-3.5" y2="1.3" width="0.127" layer="21" />
              <text x="-3.5" y="1.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="TESTPAD">
              <description>1.5 mm round test pad.</description>
              <smd name="1" x="0" y="0" dx="1.5" dy="1.5" layer="1" roundness="100" />
              <text x="-1" y="1.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="WAVESHARE-ESP32-P4-WIFI6">
              <description>Top view, USB-C end at the top, Raspberry Pi Pico pin pattern. The far end holds the Wi-Fi antenna: keep the carrier's copper clear of it, or let it overhang the edge. The P4, the microSD slot and the speaker plug are on the board's underside, so seat it on sockets. The hole circles (documentation layer) are measured, +/-0.1 mm: check before drilling.</description>
              <pad name="1" x="-8.89" y="24.13" drill="1" diameter="1.8" shape="square" />
              <pad name="40" x="8.89" y="24.13" drill="1" diameter="1.8" />
              <pad name="2" x="-8.89" y="21.59" drill="1" diameter="1.8" />
              <pad name="39" x="8.89" y="21.59" drill="1" diameter="1.8" />
              <pad name="3" x="-8.89" y="19.05" drill="1" diameter="1.8" />
              <pad name="38" x="8.89" y="19.05" drill="1" diameter="1.8" />
              <pad name="4" x="-8.89" y="16.51" drill="1" diameter="1.8" />
              <pad name="37" x="8.89" y="16.51" drill="1" diameter="1.8" />
              <pad name="5" x="-8.89" y="13.97" drill="1" diameter="1.8" />
              <pad name="36" x="8.89" y="13.97" drill="1" diameter="1.8" />
              <pad name="6" x="-8.89" y="11.43" drill="1" diameter="1.8" />
              <pad name="35" x="8.89" y="11.43" drill="1" diameter="1.8" />
              <pad name="7" x="-8.89" y="8.89" drill="1" diameter="1.8" />
              <pad name="34" x="8.89" y="8.89" drill="1" diameter="1.8" />
              <pad name="8" x="-8.89" y="6.35" drill="1" diameter="1.8" />
              <pad name="33" x="8.89" y="6.35" drill="1" diameter="1.8" />
              <pad name="9" x="-8.89" y="3.81" drill="1" diameter="1.8" />
              <pad name="32" x="8.89" y="3.81" drill="1" diameter="1.8" />
              <pad name="10" x="-8.89" y="1.27" drill="1" diameter="1.8" />
              <pad name="31" x="8.89" y="1.27" drill="1" diameter="1.8" />
              <pad name="11" x="-8.89" y="-1.27" drill="1" diameter="1.8" />
              <pad name="30" x="8.89" y="-1.27" drill="1" diameter="1.8" />
              <pad name="12" x="-8.89" y="-3.81" drill="1" diameter="1.8" />
              <pad name="29" x="8.89" y="-3.81" drill="1" diameter="1.8" />
              <pad name="13" x="-8.89" y="-6.35" drill="1" diameter="1.8" />
              <pad name="28" x="8.89" y="-6.35" drill="1" diameter="1.8" />
              <pad name="14" x="-8.89" y="-8.89" drill="1" diameter="1.8" />
              <pad name="27" x="8.89" y="-8.89" drill="1" diameter="1.8" />
              <pad name="15" x="-8.89" y="-11.43" drill="1" diameter="1.8" />
              <pad name="26" x="8.89" y="-11.43" drill="1" diameter="1.8" />
              <pad name="16" x="-8.89" y="-13.97" drill="1" diameter="1.8" />
              <pad name="25" x="8.89" y="-13.97" drill="1" diameter="1.8" />
              <pad name="17" x="-8.89" y="-16.51" drill="1" diameter="1.8" />
              <pad name="24" x="8.89" y="-16.51" drill="1" diameter="1.8" />
              <pad name="18" x="-8.89" y="-19.05" drill="1" diameter="1.8" />
              <pad name="23" x="8.89" y="-19.05" drill="1" diameter="1.8" />
              <pad name="19" x="-8.89" y="-21.59" drill="1" diameter="1.8" />
              <pad name="22" x="8.89" y="-21.59" drill="1" diameter="1.8" />
              <pad name="20" x="-8.89" y="-24.13" drill="1" diameter="1.8" />
              <pad name="21" x="8.89" y="-24.13" drill="1" diameter="1.8" />
              <circle x="-9.13" y="27.2" radius="0.85" width="0.127" layer="51" />
              <circle x="-9.13" y="-26.94" radius="0.85" width="0.127" layer="51" />
              <circle x="9.13" y="27.2" radius="0.85" width="0.127" layer="51" />
              <circle x="9.13" y="-26.94" radius="0.85" width="0.127" layer="51" />
              <wire x1="-10.5" y1="28.65" x2="10.5" y2="28.65" width="0.127" layer="21" />
              <wire x1="10.5" y1="28.65" x2="10.5" y2="-42.4" width="0.127" layer="21" />
              <wire x1="10.5" y1="-42.4" x2="6.92" y2="-42.4" width="0.127" layer="21" />
              <wire x1="6.92" y1="-42.4" x2="6.92" y2="-37.19" width="0.127" layer="21" />
              <wire x1="6.92" y1="-37.19" x2="-6.92" y2="-37.19" width="0.127" layer="21" />
              <wire x1="-6.92" y1="-37.19" x2="-6.92" y2="-42.4" width="0.127" layer="21" />
              <wire x1="-6.92" y1="-42.4" x2="-10.5" y2="-42.4" width="0.127" layer="21" />
              <wire x1="-10.5" y1="-42.4" x2="-10.5" y2="28.65" width="0.127" layer="21" />
              <text x="-10.5" y="29.2" size="1.016" layer="25">&gt;NAME</text>
            </package>
            <package name="ADAFRUIT-ULTIMATE-GPS">
              <description>1 x 9 socket for the Adafruit Ultimate GPS breakout, and 2.7 mm holes for M2.5 standoffs under its own two holes. Top view, header row at the bottom: the breakout lies over this area, component side up, with its u.FL socket near the hole above pin 1.</description>
              <pad name="1" x="10.16" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="7.62" y="0" drill="1" diameter="1.8" />
              <pad name="3" x="5.08" y="0" drill="1" diameter="1.8" />
              <pad name="4" x="2.54" y="0" drill="1" diameter="1.8" />
              <pad name="5" x="0" y="0" drill="1" diameter="1.8" />
              <pad name="6" x="-2.54" y="0" drill="1" diameter="1.8" />
              <pad name="7" x="-5.08" y="0" drill="1" diameter="1.8" />
              <pad name="8" x="-7.62" y="0" drill="1" diameter="1.8" />
              <pad name="9" x="-10.16" y="0" drill="1" diameter="1.8" />
              <hole x="-10.16" y="29.718" drill="2.7" />
              <hole x="10.16" y="29.718" drill="2.7" />
              <wire x1="-12.7" y1="32.258" x2="12.7" y2="32.258" width="0.127" layer="21" />
              <wire x1="12.7" y1="32.258" x2="12.7" y2="-2.032" width="0.127" layer="21" />
              <wire x1="12.7" y1="-2.032" x2="-12.7" y2="-2.032" width="0.127" layer="21" />
              <wire x1="-12.7" y1="-2.032" x2="-12.7" y2="32.258" width="0.127" layer="21" />
              <text x="-12.7" y="32.8" size="1.016" layer="25">&gt;NAME</text>
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
              <description>Pads in the module's order EN, VIN, GND, GND, VOUT (checked 2026-10-27). Row position within the outline from Pololu's D36V28Fx drill guide.</description>
              <pad name="EN" x="-5.08" y="0" drill="1" diameter="1.8" />
              <pad name="VIN" x="-2.54" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="GND1" x="0" y="0" drill="1" diameter="1.8" />
              <pad name="GND2" x="2.54" y="0" drill="1" diameter="1.8" />
              <pad name="VOUT" x="5.08" y="0" drill="1" diameter="1.8" />
              <wire x1="-8.9" y1="10.2" x2="8.9" y2="10.2" width="0.127" layer="21" />
              <wire x1="8.9" y1="10.2" x2="8.9" y2="-10.2" width="0.127" layer="21" />
              <wire x1="8.9" y1="-10.2" x2="-8.9" y2="-10.2" width="0.127" layer="21" />
              <wire x1="-8.9" y1="-10.2" x2="-8.9" y2="10.2" width="0.127" layer="21" />
              <text x="-8.9" y="10.7" size="1.016" layer="25">&gt;NAME</text>
            </package>
          </packages>
          <symbols>
            <symbol name="2N7002">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="G" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="D" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="S" x="10.16" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="ADAFRUIT-ULTIMATE-GPS">
              <wire x1="-10.16" y1="24.13" x2="10.16" y2="24.13" width="0.254" layer="94" />
              <wire x1="10.16" y1="24.13" x2="10.16" y2="-24.13" width="0.254" layer="94" />
              <wire x1="10.16" y1="-24.13" x2="-10.16" y2="-24.13" width="0.254" layer="94" />
              <wire x1="-10.16" y1="-24.13" x2="-10.16" y2="24.13" width="0.254" layer="94" />
              <pin name="PPS" x="-15.24" y="20.32" visible="pin" length="middle" direction="pas" />
              <pin name="VIN" x="-15.24" y="15.24" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-15.24" y="10.16" visible="pin" length="middle" direction="pas" />
              <pin name="RX" x="-15.24" y="5.08" visible="pin" length="middle" direction="pas" />
              <pin name="TX" x="-15.24" y="0" visible="pin" length="middle" direction="pas" />
              <pin name="FIX" x="-15.24" y="-5.08" visible="pin" length="middle" direction="pas" />
              <pin name="VBAT" x="-15.24" y="-10.16" visible="pin" length="middle" direction="pas" />
              <pin name="EN" x="-15.24" y="-15.24" visible="pin" length="middle" direction="pas" />
              <pin name="3V3" x="-15.24" y="-20.32" visible="pin" length="middle" direction="pas" />
              <text x="-10.16" y="25.13" size="1.778" layer="95">&gt;NAME</text>
              <text x="-10.16" y="-26.93" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="BAT54S">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="A1" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="K2" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="COM" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
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
            <symbol name="HEADER-1X3">
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
            <symbol name="HEADER-1X6">
              <wire x1="-5.08" y1="16.51" x2="5.08" y2="16.51" width="0.254" layer="94" />
              <wire x1="5.08" y1="16.51" x2="5.08" y2="-16.51" width="0.254" layer="94" />
              <wire x1="5.08" y1="-16.51" x2="-5.08" y2="-16.51" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-16.51" x2="-5.08" y2="16.51" width="0.254" layer="94" />
              <pin name="1" x="-10.16" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="2" x="-10.16" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="3" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="4" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="5" x="-10.16" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="6" x="-10.16" y="-12.7" visible="pin" length="middle" direction="pas" />
              <text x="-5.08" y="17.51" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-19.31" size="1.778" layer="96">&gt;VALUE</text>
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
            <symbol name="MCP4531">
              <wire x1="-10.16" y1="11.43" x2="10.16" y2="11.43" width="0.254" layer="94" />
              <wire x1="10.16" y1="11.43" x2="10.16" y2="-11.43" width="0.254" layer="94" />
              <wire x1="10.16" y1="-11.43" x2="-10.16" y2="-11.43" width="0.254" layer="94" />
              <wire x1="-10.16" y1="-11.43" x2="-10.16" y2="11.43" width="0.254" layer="94" />
              <pin name="VDD" x="-15.24" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="SCL" x="-15.24" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="SDA" x="-15.24" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VSS" x="-15.24" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="A0" x="15.24" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="P0A" x="15.24" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="P0W" x="15.24" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="P0B" x="15.24" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-10.16" y="12.43" size="1.778" layer="95">&gt;NAME</text>
              <text x="-10.16" y="-14.23" size="1.778" layer="96">&gt;VALUE</text>
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
            <symbol name="MICROFIT-2X3">
              <wire x1="-5.08" y1="16.51" x2="5.08" y2="16.51" width="0.254" layer="94" />
              <wire x1="5.08" y1="16.51" x2="5.08" y2="-16.51" width="0.254" layer="94" />
              <wire x1="5.08" y1="-16.51" x2="-5.08" y2="-16.51" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-16.51" x2="-5.08" y2="16.51" width="0.254" layer="94" />
              <pin name="1" x="-10.16" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="2" x="-10.16" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="3" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="4" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="5" x="-10.16" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="6" x="-10.16" y="-12.7" visible="pin" length="middle" direction="pas" />
              <text x="-5.08" y="17.51" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-19.31" size="1.778" layer="96">&gt;VALUE</text>
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
            <symbol name="PFET">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="G" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="S" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="D" x="10.16" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="PNP">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="B" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="E" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="C" x="10.16" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
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
            <symbol name="TL431">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="REF" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="A" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="K" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
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
            <symbol name="WAVESHARE-ESP32-P4-WIFI6">
              <wire x1="-15.24" y1="52.07" x2="15.24" y2="52.07" width="0.254" layer="94" />
              <wire x1="15.24" y1="52.07" x2="15.24" y2="-52.07" width="0.254" layer="94" />
              <wire x1="15.24" y1="-52.07" x2="-15.24" y2="-52.07" width="0.254" layer="94" />
              <wire x1="-15.24" y1="-52.07" x2="-15.24" y2="52.07" width="0.254" layer="94" />
              <pin name="IO52" x="-20.32" y="48.26" visible="pin" length="middle" direction="pas" />
              <pin name="IO51" x="-20.32" y="43.18" visible="pin" length="middle" direction="pas" />
              <pin name="GND@1" x="-20.32" y="38.1" visible="pin" length="middle" direction="pas" />
              <pin name="IO31" x="-20.32" y="33.02" visible="pin" length="middle" direction="pas" />
              <pin name="IO30" x="-20.32" y="27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO29" x="-20.32" y="22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO28" x="-20.32" y="17.78" visible="pin" length="middle" direction="pas" />
              <pin name="GND@2" x="-20.32" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO50" x="-20.32" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO49" x="-20.32" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO5" x="-20.32" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO4" x="-20.32" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="GND@3" x="-20.32" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO3" x="-20.32" y="-17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO2" x="-20.32" y="-22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO8_SCL" x="-20.32" y="-27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO7_SDA" x="-20.32" y="-33.02" visible="pin" length="middle" direction="pas" />
              <pin name="GND@4" x="-20.32" y="-38.1" visible="pin" length="middle" direction="pas" />
              <pin name="IO24_USB_DM" x="-20.32" y="-43.18" visible="pin" length="middle" direction="pas" />
              <pin name="IO25_USB_DP" x="-20.32" y="-48.26" visible="pin" length="middle" direction="pas" />
              <pin name="VBUS" x="20.32" y="48.26" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="VSYS" x="20.32" y="43.18" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@8" x="20.32" y="38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="3V3_EN" x="20.32" y="33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="3V3" x="20.32" y="27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO20" x="20.32" y="22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO21" x="20.32" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@7" x="20.32" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO22" x="20.32" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO23" x="20.32" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="RUN" x="20.32" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO26" x="20.32" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@6" x="20.32" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO27" x="20.32" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO32" x="20.32" y="-22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO33" x="20.32" y="-27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO46" x="20.32" y="-33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@5" x="20.32" y="-38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO47" x="20.32" y="-43.18" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO48" x="20.32" y="-48.26" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-15.24" y="53.07" size="1.778" layer="95">&gt;NAME</text>
              <text x="-15.24" y="-54.87" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
            <symbol name="XC6206">
              <wire x1="-5.08" y1="6.35" x2="5.08" y2="6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="6.35" x2="5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="5.08" y1="-6.35" x2="-5.08" y2="-6.35" width="0.254" layer="94" />
              <wire x1="-5.08" y1="-6.35" x2="-5.08" y2="6.35" width="0.254" layer="94" />
              <pin name="VIN" x="-10.16" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="GND" x="-10.16" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="VOUT" x="10.16" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-5.08" y="7.35" size="1.778" layer="95">&gt;NAME</text>
              <text x="-5.08" y="-9.15" size="1.778" layer="96">&gt;VALUE</text>
            </symbol>
          </symbols>
          <devicesets>
            <deviceset name="2N7002" prefix="Q" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="2N7002" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="G" pad="1" />
                    <connect gate="G$1" pin="S" pad="2" />
                    <connect gate="G$1" pin="D" pad="3" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="ADAFRUIT-ULTIMATE-GPS" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="ADAFRUIT-ULTIMATE-GPS" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="ADAFRUIT-ULTIMATE-GPS">
                  <connects>
                    <connect gate="G$1" pin="PPS" pad="1" />
                    <connect gate="G$1" pin="VIN" pad="2" />
                    <connect gate="G$1" pin="GND" pad="3" />
                    <connect gate="G$1" pin="RX" pad="4" />
                    <connect gate="G$1" pin="TX" pad="5" />
                    <connect gate="G$1" pin="FIX" pad="6" />
                    <connect gate="G$1" pin="VBAT" pad="7" />
                    <connect gate="G$1" pin="EN" pad="8" />
                    <connect gate="G$1" pin="3V3" pad="9" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="BAS70" prefix="D" uservalue="yes">
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
            <deviceset name="BAT54S" prefix="D" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="BAT54S" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="A1" pad="1" />
                    <connect gate="G$1" pin="K2" pad="2" />
                    <connect gate="G$1" pin="COM" pad="3" />
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
            <deviceset name="HEADER-1X3" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="HEADER-1X3" x="0" y="0" />
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
            <deviceset name="HEADER-1X6" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="HEADER-1X6" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X6">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                    <connect gate="G$1" pin="3" pad="3" />
                    <connect gate="G$1" pin="4" pad="4" />
                    <connect gate="G$1" pin="5" pad="5" />
                    <connect gate="G$1" pin="6" pad="6" />
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
            <deviceset name="MCP4531-MSOP" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="MCP4531" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="MSOP8">
                  <connects>
                    <connect gate="G$1" pin="VDD" pad="1" />
                    <connect gate="G$1" pin="SCL" pad="2" />
                    <connect gate="G$1" pin="SDA" pad="3" />
                    <connect gate="G$1" pin="VSS" pad="4" />
                    <connect gate="G$1" pin="P0B" pad="5" />
                    <connect gate="G$1" pin="P0W" pad="6" />
                    <connect gate="G$1" pin="P0A" pad="7" />
                    <connect gate="G$1" pin="A0" pad="8" />
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
            <deviceset name="MICROFIT-2X3" prefix="J" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="MICROFIT-2X3" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="MOLEX-43045-0612">
                  <connects>
                    <connect gate="G$1" pin="1" pad="1" />
                    <connect gate="G$1" pin="2" pad="2" />
                    <connect gate="G$1" pin="3" pad="3" />
                    <connect gate="G$1" pin="4" pad="4" />
                    <connect gate="G$1" pin="5" pad="5" />
                    <connect gate="G$1" pin="6" pad="6" />
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
            <deviceset name="PFET-SOT23" prefix="Q" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="PFET" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="G" pad="1" />
                    <connect gate="G$1" pin="S" pad="2" />
                    <connect gate="G$1" pin="D" pad="3" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="PNP-SOT23" prefix="Q" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="PNP" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="B" pad="1" />
                    <connect gate="G$1" pin="E" pad="2" />
                    <connect gate="G$1" pin="C" pad="3" />
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
            <deviceset name="R-AXIAL" prefix="R" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="RESISTOR" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="AXIAL-10MM">
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
            <deviceset name="TL431-SOT23" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="TL431" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="REF" pad="1" />
                    <connect gate="G$1" pin="A" pad="2" />
                    <connect gate="G$1" pin="K" pad="3" />
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
            <deviceset name="WAVESHARE-ESP32-P4-WIFI6" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="WAVESHARE-ESP32-P4-WIFI6" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="WAVESHARE-ESP32-P4-WIFI6">
                  <connects>
                    <connect gate="G$1" pin="IO52" pad="1" />
                    <connect gate="G$1" pin="IO51" pad="2" />
                    <connect gate="G$1" pin="GND@1" pad="3" />
                    <connect gate="G$1" pin="IO31" pad="4" />
                    <connect gate="G$1" pin="IO30" pad="5" />
                    <connect gate="G$1" pin="IO29" pad="6" />
                    <connect gate="G$1" pin="IO28" pad="7" />
                    <connect gate="G$1" pin="GND@2" pad="8" />
                    <connect gate="G$1" pin="IO50" pad="9" />
                    <connect gate="G$1" pin="IO49" pad="10" />
                    <connect gate="G$1" pin="IO5" pad="11" />
                    <connect gate="G$1" pin="IO4" pad="12" />
                    <connect gate="G$1" pin="GND@3" pad="13" />
                    <connect gate="G$1" pin="IO3" pad="14" />
                    <connect gate="G$1" pin="IO2" pad="15" />
                    <connect gate="G$1" pin="IO8_SCL" pad="16" />
                    <connect gate="G$1" pin="IO7_SDA" pad="17" />
                    <connect gate="G$1" pin="GND@4" pad="18" />
                    <connect gate="G$1" pin="IO24_USB_DM" pad="19" />
                    <connect gate="G$1" pin="IO25_USB_DP" pad="20" />
                    <connect gate="G$1" pin="VBUS" pad="40" />
                    <connect gate="G$1" pin="VSYS" pad="39" />
                    <connect gate="G$1" pin="GND@8" pad="38" />
                    <connect gate="G$1" pin="3V3_EN" pad="37" />
                    <connect gate="G$1" pin="3V3" pad="36" />
                    <connect gate="G$1" pin="IO20" pad="35" />
                    <connect gate="G$1" pin="IO21" pad="34" />
                    <connect gate="G$1" pin="GND@7" pad="33" />
                    <connect gate="G$1" pin="IO22" pad="32" />
                    <connect gate="G$1" pin="IO23" pad="31" />
                    <connect gate="G$1" pin="RUN" pad="30" />
                    <connect gate="G$1" pin="IO26" pad="29" />
                    <connect gate="G$1" pin="GND@6" pad="28" />
                    <connect gate="G$1" pin="IO27" pad="27" />
                    <connect gate="G$1" pin="IO32" pad="26" />
                    <connect gate="G$1" pin="IO33" pad="25" />
                    <connect gate="G$1" pin="IO46" pad="24" />
                    <connect gate="G$1" pin="GND@5" pad="23" />
                    <connect gate="G$1" pin="IO47" pad="22" />
                    <connect gate="G$1" pin="IO48" pad="21" />
                  </connects>
                  <technologies>
                    <technology name="" />
                  </technologies>
                </device>
              </devices>
            </deviceset>
            <deviceset name="XC6206" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="XC6206" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="SOT23">
                  <connects>
                    <connect gate="G$1" pin="GND" pad="1" />
                    <connect gate="G$1" pin="VOUT" pad="2" />
                    <connect gate="G$1" pin="VIN" pad="3" />
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
        <part name="TP6" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+5V_RAW" />
        <part name="Q7" library="haltech-gauges" deviceset="PFET-SOT23" device="" value="AO3401A">
          <attribute name="LCSC" value="C15127" />
        </part>
        <part name="Q8" library="haltech-gauges" deviceset="PNP-SOT23" device="" value="MMBT3906" />
        <part name="U9" library="haltech-gauges" deviceset="TL431-SOT23" device="" value="TL431">
          <attribute name="LCSC" value="C3113" />
        </part>
        <part name="R40" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="R41" library="haltech-gauges" deviceset="R0805" device="" value="2k2">
          <attribute name="LCSC" value="C17520" />
        </part>
        <part name="R42" library="haltech-gauges" deviceset="R0805" device="" value="15k" />
        <part name="R43" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="C19" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C5" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="D3" library="haltech-gauges" deviceset="SCHOTTKY-SMA" device="" value="SS14">
          <attribute name="LCSC" value="C2480" />
        </part>
        <part name="F2" library="haltech-gauges" deviceset="POLYFUSE-1812" device="" value="1812L200/16DR">
          <attribute name="LCSC" value="C439873" />
        </part>
        <part name="J11" library="haltech-gauges" deviceset="MICROFIT-2X3" device="" value="43045-0612 CHAIN" />
        <part name="R44" library="haltech-gauges" deviceset="R0805" device="" value="4k7">
          <attribute name="LCSC" value="C17673" />
        </part>
        <part name="U4" library="haltech-gauges" deviceset="WAVESHARE-ESP32-P4-WIFI6" device="" value="ESP32-P4-WIFI6" />
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
        <part name="U7" library="haltech-gauges" deviceset="TJA1051T-3" device="" value="TJA1051T/3">
          <attribute name="LCSC" value="C38695" />
        </part>
        <part name="D7" library="haltech-gauges" deviceset="PESD2CAN" device="" value="PESD2CAN">
          <attribute name="LCSC" value="C75176" />
        </part>
        <part name="R46" library="haltech-gauges" deviceset="R1206" device="" value="120R">
          <attribute name="LCSC" value="C17909" />
        </part>
        <part name="JP3" library="haltech-gauges" deviceset="JUMPER-2" device="" value="VEHICLE TERM" />
        <part name="J14" library="haltech-gauges" deviceset="MICROFIT-2X3" device="" value="43045-0612 VEHICLE CAN" />
        <part name="R45" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="C20" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="C23" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="J12" library="haltech-gauges" deviceset="ADAFRUIT-ULTIMATE-GPS" device="" value="ADAFRUIT 746 SOCKET" />
        <part name="Q1" library="haltech-gauges" deviceset="2N7002" device="" value="2N7002">
          <attribute name="LCSC" value="C8545" />
        </part>
        <part name="R24" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R22" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="R23" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="C16" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
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
        <part name="R8" library="haltech-gauges" deviceset="R0805" device="" value="150k" />
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
        <part name="BT1" library="haltech-gauges" deviceset="BATTERY-2032" device="" value="ML2032 holder" />
        <part name="C15" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="U6" library="haltech-gauges" deviceset="XC6206" device="" value="XC6206P302MR" />
        <part name="C17" library="haltech-gauges" deviceset="C0805" device="" value="1u 50V">
          <attribute name="LCSC" value="C28323" />
        </part>
        <part name="C18" library="haltech-gauges" deviceset="C0805" device="" value="1u 50V">
          <attribute name="LCSC" value="C28323" />
        </part>
        <part name="D6" library="haltech-gauges" deviceset="BAS70" device="" value="BAS70" />
        <part name="R21" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="JP2" library="haltech-gauges" deviceset="JUMPER-2" device="" value="ML2032 CHARGE" />
        <part name="J15" library="haltech-gauges" deviceset="HEADER-1X3" device="" value="RELAY 5V GND IN" />
        <part name="Q2" library="haltech-gauges" deviceset="2N7002" device="" value="AO3400A">
          <attribute name="LCSC" value="C20917" />
        </part>
        <part name="C21" library="haltech-gauges" deviceset="C0805" device="" value="1u 50V">
          <attribute name="LCSC" value="C28323" />
        </part>
        <part name="D8" library="haltech-gauges" deviceset="BAT54S" device="" value="BAT54S">
          <attribute name="LCSC" value="C47546" />
        </part>
        <part name="C22" library="haltech-gauges" deviceset="C0805" device="" value="10u 25V">
          <attribute name="LCSC" value="C15850" />
        </part>
        <part name="R47" library="haltech-gauges" deviceset="R0805" device="" value="4k7">
          <attribute name="LCSC" value="C17673" />
        </part>
        <part name="R48" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R49" library="haltech-gauges" deviceset="R0805" device="" value="1k">
          <attribute name="LCSC" value="C17513" />
        </part>
        <part name="JP4" library="haltech-gauges" deviceset="JUMPER-2" device="" value="HORN DIRECT" />
        <part name="J13" library="haltech-gauges" deviceset="HEADER-1X6" device="" value="REMOTE GND REM SW1-4" />
        <part name="Q3" library="haltech-gauges" deviceset="2N7002" device="" value="2N7002">
          <attribute name="LCSC" value="C8545" />
        </part>
        <part name="Q4" library="haltech-gauges" deviceset="2N7002" device="" value="2N7002">
          <attribute name="LCSC" value="C8545" />
        </part>
        <part name="Q5" library="haltech-gauges" deviceset="2N7002" device="" value="2N7002">
          <attribute name="LCSC" value="C8545" />
        </part>
        <part name="Q6" library="haltech-gauges" deviceset="2N7002" device="" value="2N7002">
          <attribute name="LCSC" value="C8545" />
        </part>
        <part name="R30" library="haltech-gauges" deviceset="R-AXIAL" device="" value="DNP head unit" />
        <part name="R31" library="haltech-gauges" deviceset="R-AXIAL" device="" value="DNP head unit" />
        <part name="R32" library="haltech-gauges" deviceset="R-AXIAL" device="" value="DNP head unit" />
        <part name="R33" library="haltech-gauges" deviceset="R-AXIAL" device="" value="DNP head unit" />
        <part name="R50" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R51" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R52" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="R53" library="haltech-gauges" deviceset="R0805" device="" value="100k">
          <attribute name="LCSC" value="C149504" />
        </part>
        <part name="U8" library="haltech-gauges" deviceset="MCP4531-MSOP" device="" value="MCP4531T-103E/MS DNP" />
        <part name="C24" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="TP1" library="haltech-gauges" deviceset="TESTPOINT" device="" value="GND" />
        <part name="TP2" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+5V" />
        <part name="TP3" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+3V3" />
        <part name="TP4" library="haltech-gauges" deviceset="TESTPOINT" device="" value="VBAT_SENSE" />
        <part name="TP5" library="haltech-gauges" deviceset="TESTPOINT" device="" value="RST" />
      </parts>
      <sheets>
        <sheet>
          <plain>
            <text x="10.16" y="441.48" size="2.54" layer="97">Haltech gauges: hub carrier rev B, ESP32-P4 (Waveshare ESP32-P4-WIFI6)</text>
            <text x="10.16" y="435.48" size="1.778" layer="97">J10 ECU, the only 2x2: 1 +12V switched, 2 GND, 3 CAN H, 4 CAN L. J11 gauge chain, 2x3: 1 +5V, 2 GND, 3 CAN H, 4 CAN L, 5 sense, 6 GND. J14 vehicle CAN, 2x3: 1 NC, 2 GND, 3 CAN H, 4 CAN L, 5 NC, 6 GND.</text>
            <text x="10.16" y="432.48" size="1.778" layer="97">U1 Pololu D36V28F5, pins EN, VIN, GND, GND, VOUT left to right (checked against a module 2026-10-27). Its output is +5V_RAW; Q7 passes it to +5V only while U9 (TL431, trip 6.24 V) sees it under the limit, so a wrong module, socket or supply cannot reach the board or the chain.</text>
            <text x="10.16" y="429.48" size="1.778" layer="97">U4 Waveshare ESP32-P4-WIFI6 (SKU 32020) on two 1x20 sockets. 5 V reaches VSYS through D3; never connect VBUS. Keep the carrier's copper away from the antenna end.</text>
            <text x="10.16" y="426.48" size="1.778" layer="97">U2 ECU transceiver (receive only in firmware), U3 gauge bus, U7 vehicle CAN to the Nexus Rebel's CAN 2 in FG Falcon mode: GPIO26 TX, GPIO27 RX, GPIO52 silent (listen only when high). JP3 is the 120 R of the two-node vehicle bus: fitted.</text>
            <text x="10.16" y="423.48" size="1.778" layer="97">Horn: J15 takes an opto-isolated relay module set to low-level trigger; Q2 pulls its IN low. GPIO49 drives Q2 through a charge pump (C21, D8, C22, R47): only a 1 kHz square wave holds the relay and a stuck level drops it in about 50 ms (firmware HG_HORN_PULSED). JP4 bypasses the pump for a plain level drive through R49.</text>
            <text x="10.16" y="420.48" size="1.778" layer="97">Wired remote for the head unit: J13 1x6 (GND, REM, SW1-SW4). Q3-Q6 switch R30-R33 (through-hole, unfitted until the head unit is known) to REM from GPIO2-5. U8 MCP4531 (DNP) is the digital-pot alternative on the board's I2C, wiper to REM.</text>
            <text x="10.16" y="417.48" size="1.778" layer="97">Clock U5 on GPIO7/8 with INT on GPIO31; BT1 ML2032 ONLY, charged through U6 (3.0 V), D6 and R21 when JP2 is fitted. GPS socket J12: GPIO28 in, 29 out, 30 PPS, 50 off (Q1); R24 keeps the GPS on while the P4 is in reset.</text>
            <text x="10.16" y="414.48" size="1.778" layer="97">J11 pin 5 (chain sense) goes to GPIO51 with R44's pull-up: low while a chain plug is in. Not used by the firmware yet.</text>
            <text x="10.16" y="411.48" size="1.778" layer="97">Footprints are generic or placeholders except U4 and J12: replace them with the LCSC parts' own before layout. LCSC numbers marked unchecked in docs/bom-jlcpcb.md are to be confirmed in EasyEDA.</text>
          </plain>
          <instances>
            <instance part="J10" gate="G$1" x="35.56" y="381" />
            <instance part="D1" gate="G$1" x="86.36" y="381" />
            <instance part="D2" gate="G$1" x="137.16" y="381" />
            <instance part="C1" gate="G$1" x="187.96" y="381" />
            <instance part="C2" gate="G$1" x="238.76" y="381" />
            <instance part="U1" gate="G$1" x="45.72" y="345.44" />
            <instance part="C3" gate="G$1" x="116.84" y="355.6" />
            <instance part="C4" gate="G$1" x="167.64" y="355.6" />
            <instance part="TP6" gate="G$1" x="218.44" y="355.6" />
            <instance part="Q7" gate="G$1" x="116.84" y="332.74" />
            <instance part="Q8" gate="G$1" x="167.64" y="332.74" />
            <instance part="U9" gate="G$1" x="218.44" y="332.74" />
            <instance part="R40" gate="G$1" x="269.24" y="355.6" />
            <instance part="R41" gate="G$1" x="269.24" y="342.9" />
            <instance part="R42" gate="G$1" x="269.24" y="330.2" />
            <instance part="R43" gate="G$1" x="320.04" y="355.6" />
            <instance part="C19" gate="G$1" x="320.04" y="342.9" />
            <instance part="C5" gate="G$1" x="320.04" y="330.2" />
            <instance part="D3" gate="G$1" x="116.84" y="314.96" />
            <instance part="F2" gate="G$1" x="177.8" y="314.96" />
            <instance part="J11" gate="G$1" x="279.4" y="309.88" />
            <instance part="R44" gate="G$1" x="330.2" y="314.96" />
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
            <instance part="U7" gate="G$1" x="182.88" y="193.04" />
            <instance part="D7" gate="G$1" x="254" y="198.12" />
            <instance part="R46" gate="G$1" x="254" y="185.42" />
            <instance part="JP3" gate="G$1" x="304.8" y="185.42" />
            <instance part="J14" gate="G$1" x="355.6" y="193.04" />
            <instance part="R45" gate="G$1" x="254" y="172.72" />
            <instance part="C20" gate="G$1" x="304.8" y="172.72" />
            <instance part="C23" gate="G$1" x="355.6" y="172.72" />
            <instance part="J12" gate="G$1" x="436.88" y="284.48" />
            <instance part="Q1" gate="G$1" x="431.8" y="241.3" />
            <instance part="R24" gate="G$1" x="431.8" y="223.52" />
            <instance part="R22" gate="G$1" x="431.8" y="210.82" />
            <instance part="R23" gate="G$1" x="431.8" y="198.12" />
            <instance part="C16" gate="G$1" x="431.8" y="185.42" />
            <instance part="C6" gate="G$1" x="157.48" y="157.48" />
            <instance part="C7" gate="G$1" x="213.36" y="157.48" />
            <instance part="C8" gate="G$1" x="269.24" y="157.48" />
            <instance part="C10" gate="G$1" x="325.12" y="157.48" />
            <instance part="R1" gate="G$1" x="157.48" y="144.78" />
            <instance part="R2" gate="G$1" x="213.36" y="144.78" />
            <instance part="R8" gate="G$1" x="269.24" y="144.78" />
            <instance part="R9" gate="G$1" x="325.12" y="144.78" />
            <instance part="U5" gate="G$1" x="76.2" y="111.76" />
            <instance part="C14" gate="G$1" x="157.48" y="127" />
            <instance part="R20" gate="G$1" x="213.36" y="127" />
            <instance part="BT1" gate="G$1" x="157.48" y="109.22" />
            <instance part="C15" gate="G$1" x="213.36" y="109.22" />
            <instance part="U6" gate="G$1" x="269.24" y="116.84" />
            <instance part="C17" gate="G$1" x="325.12" y="127" />
            <instance part="C18" gate="G$1" x="325.12" y="109.22" />
            <instance part="D6" gate="G$1" x="157.48" y="91.44" />
            <instance part="R21" gate="G$1" x="213.36" y="91.44" />
            <instance part="JP2" gate="G$1" x="269.24" y="91.44" />
            <instance part="J15" gate="G$1" x="35.56" y="55.88" />
            <instance part="Q2" gate="G$1" x="86.36" y="55.88" />
            <instance part="C21" gate="G$1" x="137.16" y="60.96" />
            <instance part="D8" gate="G$1" x="187.96" y="55.88" />
            <instance part="C22" gate="G$1" x="238.76" y="60.96" />
            <instance part="R47" gate="G$1" x="238.76" y="48.26" />
            <instance part="R48" gate="G$1" x="137.16" y="48.26" />
            <instance part="R49" gate="G$1" x="289.56" y="60.96" />
            <instance part="JP4" gate="G$1" x="289.56" y="48.26" />
            <instance part="J13" gate="G$1" x="381" y="127" />
            <instance part="Q3" gate="G$1" x="381" y="91.44" />
            <instance part="Q4" gate="G$1" x="421.64" y="91.44" />
            <instance part="Q5" gate="G$1" x="462.28" y="91.44" />
            <instance part="Q6" gate="G$1" x="502.92" y="91.44" />
            <instance part="R30" gate="G$1" x="381" y="73.66" />
            <instance part="R31" gate="G$1" x="421.64" y="73.66" />
            <instance part="R32" gate="G$1" x="462.28" y="73.66" />
            <instance part="R33" gate="G$1" x="502.92" y="73.66" />
            <instance part="R50" gate="G$1" x="381" y="60.96" />
            <instance part="R51" gate="G$1" x="421.64" y="60.96" />
            <instance part="R52" gate="G$1" x="462.28" y="60.96" />
            <instance part="R53" gate="G$1" x="502.92" y="60.96" />
            <instance part="U8" gate="G$1" x="381" y="25.4" />
            <instance part="C24" gate="G$1" x="462.28" y="30.48" />
            <instance part="TP1" gate="G$1" x="436.88" y="381" />
            <instance part="TP2" gate="G$1" x="436.88" y="370.84" />
            <instance part="TP3" gate="G$1" x="436.88" y="360.68" />
            <instance part="TP4" gate="G$1" x="436.88" y="350.52" />
            <instance part="TP5" gate="G$1" x="436.88" y="340.36" />
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
                <wire x1="264.16" y1="144.78" x2="261.62" y2="144.78" width="0.1524" layer="91" />
                <label x="261.62" y="144.78" size="1.778" layer="95" rot="R180" />
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
            <net name="+5V_RAW" class="0">
              <segment>
                <pinref part="U1" gate="G$1" pin="VOUT" />
                <wire x1="60.96" y1="350.52" x2="63.5" y2="350.52" width="0.1524" layer="91" />
                <label x="63.5" y="350.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C3" gate="G$1" pin="+" />
                <wire x1="111.76" y1="355.6" x2="109.22" y2="355.6" width="0.1524" layer="91" />
                <label x="109.22" y="355.6" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="1" />
                <wire x1="162.56" y1="355.6" x2="160.02" y2="355.6" width="0.1524" layer="91" />
                <label x="160.02" y="355.6" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP6" gate="G$1" pin="TP" />
                <wire x1="215.9" y1="355.6" x2="213.36" y2="355.6" width="0.1524" layer="91" />
                <label x="213.36" y="355.6" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q7" gate="G$1" pin="S" />
                <wire x1="127" y1="335.28" x2="129.54" y2="335.28" width="0.1524" layer="91" />
                <label x="129.54" y="335.28" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q8" gate="G$1" pin="E" />
                <wire x1="177.8" y1="335.28" x2="180.34" y2="335.28" width="0.1524" layer="91" />
                <label x="180.34" y="335.28" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R42" gate="G$1" pin="1" />
                <wire x1="264.16" y1="330.2" x2="261.62" y2="330.2" width="0.1524" layer="91" />
                <label x="261.62" y="330.2" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="OV_SENSE" class="0">
              <segment>
                <pinref part="R42" gate="G$1" pin="2" />
                <wire x1="274.32" y1="330.2" x2="276.86" y2="330.2" width="0.1524" layer="91" />
                <label x="276.86" y="330.2" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R43" gate="G$1" pin="1" />
                <wire x1="314.96" y1="355.6" x2="312.42" y2="355.6" width="0.1524" layer="91" />
                <label x="312.42" y="355.6" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C19" gate="G$1" pin="1" />
                <wire x1="314.96" y1="342.9" x2="312.42" y2="342.9" width="0.1524" layer="91" />
                <label x="312.42" y="342.9" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U9" gate="G$1" pin="REF" />
                <wire x1="208.28" y1="335.28" x2="205.74" y2="335.28" width="0.1524" layer="91" />
                <label x="205.74" y="335.28" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="OV_K" class="0">
              <segment>
                <pinref part="U9" gate="G$1" pin="K" />
                <wire x1="228.6" y1="335.28" x2="231.14" y2="335.28" width="0.1524" layer="91" />
                <label x="231.14" y="335.28" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R41" gate="G$1" pin="1" />
                <wire x1="264.16" y1="342.9" x2="261.62" y2="342.9" width="0.1524" layer="91" />
                <label x="261.62" y="342.9" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="OV_BASE" class="0">
              <segment>
                <pinref part="R41" gate="G$1" pin="2" />
                <wire x1="274.32" y1="342.9" x2="276.86" y2="342.9" width="0.1524" layer="91" />
                <label x="276.86" y="342.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q8" gate="G$1" pin="B" />
                <wire x1="157.48" y1="335.28" x2="154.94" y2="335.28" width="0.1524" layer="91" />
                <label x="154.94" y="335.28" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="OV_GATE" class="0">
              <segment>
                <pinref part="Q8" gate="G$1" pin="C" />
                <wire x1="177.8" y1="330.2" x2="180.34" y2="330.2" width="0.1524" layer="91" />
                <label x="180.34" y="330.2" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q7" gate="G$1" pin="G" />
                <wire x1="106.68" y1="335.28" x2="104.14" y2="335.28" width="0.1524" layer="91" />
                <label x="104.14" y="335.28" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R40" gate="G$1" pin="1" />
                <wire x1="264.16" y1="355.6" x2="261.62" y2="355.6" width="0.1524" layer="91" />
                <label x="261.62" y="355.6" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V" class="0">
              <segment>
                <pinref part="Q7" gate="G$1" pin="D" />
                <wire x1="127" y1="330.2" x2="129.54" y2="330.2" width="0.1524" layer="91" />
                <label x="129.54" y="330.2" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="1" />
                <wire x1="314.96" y1="330.2" x2="312.42" y2="330.2" width="0.1524" layer="91" />
                <label x="312.42" y="330.2" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="F2" gate="G$1" pin="1" />
                <wire x1="172.72" y1="314.96" x2="170.18" y2="314.96" width="0.1524" layer="91" />
                <label x="170.18" y="314.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D3" gate="G$1" pin="A" />
                <wire x1="111.76" y1="314.96" x2="109.22" y2="314.96" width="0.1524" layer="91" />
                <label x="109.22" y="314.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VCC" />
                <wire x1="167.64" y1="276.86" x2="165.1" y2="276.86" width="0.1524" layer="91" />
                <label x="165.1" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C6" gate="G$1" pin="1" />
                <wire x1="152.4" y1="157.48" x2="149.86" y2="157.48" width="0.1524" layer="91" />
                <label x="149.86" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP2" gate="G$1" pin="TP" />
                <wire x1="434.34" y1="370.84" x2="431.8" y2="370.84" width="0.1524" layer="91" />
                <label x="431.8" y="370.84" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J12" gate="G$1" pin="VIN" />
                <wire x1="421.64" y1="299.72" x2="419.1" y2="299.72" width="0.1524" layer="91" />
                <label x="419.1" y="299.72" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C16" gate="G$1" pin="1" />
                <wire x1="426.72" y1="185.42" x2="424.18" y2="185.42" width="0.1524" layer="91" />
                <label x="424.18" y="185.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="VCC" />
                <wire x1="167.64" y1="185.42" x2="165.1" y2="185.42" width="0.1524" layer="91" />
                <label x="165.1" y="185.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C20" gate="G$1" pin="1" />
                <wire x1="299.72" y1="172.72" x2="297.18" y2="172.72" width="0.1524" layer="91" />
                <label x="297.18" y="172.72" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J15" gate="G$1" pin="1" />
                <wire x1="25.4" y1="60.96" x2="22.86" y2="60.96" width="0.1524" layer="91" />
                <label x="22.86" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V_MCU" class="0">
              <segment>
                <pinref part="D3" gate="G$1" pin="K" />
                <wire x1="121.92" y1="314.96" x2="124.46" y2="314.96" width="0.1524" layer="91" />
                <label x="124.46" y="314.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="VSYS" />
                <wire x1="96.52" y1="297.18" x2="99.06" y2="297.18" width="0.1524" layer="91" />
                <label x="99.06" y="297.18" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="+5V_CHAIN" class="0">
              <segment>
                <pinref part="F2" gate="G$1" pin="2" />
                <wire x1="182.88" y1="314.96" x2="185.42" y2="314.96" width="0.1524" layer="91" />
                <label x="185.42" y="314.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="1" />
                <wire x1="269.24" y1="322.58" x2="266.7" y2="322.58" width="0.1524" layer="91" />
                <label x="266.7" y="322.58" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+3V3" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="3V3" />
                <wire x1="96.52" y1="281.94" x2="99.06" y2="281.94" width="0.1524" layer="91" />
                <label x="99.06" y="281.94" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VIO" />
                <wire x1="167.64" y1="281.94" x2="165.1" y2="281.94" width="0.1524" layer="91" />
                <label x="165.1" y="281.94" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="1" />
                <wire x1="208.28" y1="157.48" x2="205.74" y2="157.48" width="0.1524" layer="91" />
                <label x="205.74" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="VCC" />
                <wire x1="167.64" y1="231.14" x2="165.1" y2="231.14" width="0.1524" layer="91" />
                <label x="165.1" y="231.14" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="1" />
                <wire x1="264.16" y1="157.48" x2="261.62" y2="157.48" width="0.1524" layer="91" />
                <label x="261.62" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="1" />
                <wire x1="208.28" y1="144.78" x2="205.74" y2="144.78" width="0.1524" layer="91" />
                <label x="205.74" y="144.78" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP3" gate="G$1" pin="TP" />
                <wire x1="434.34" y1="360.68" x2="431.8" y2="360.68" width="0.1524" layer="91" />
                <label x="431.8" y="360.68" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="VCC" />
                <wire x1="58.42" y1="129.54" x2="55.88" y2="129.54" width="0.1524" layer="91" />
                <label x="55.88" y="129.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C14" gate="G$1" pin="1" />
                <wire x1="152.4" y1="127" x2="149.86" y2="127" width="0.1524" layer="91" />
                <label x="149.86" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R20" gate="G$1" pin="1" />
                <wire x1="208.28" y1="127" x2="205.74" y2="127" width="0.1524" layer="91" />
                <label x="205.74" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="VIN" />
                <wire x1="259.08" y1="119.38" x2="256.54" y2="119.38" width="0.1524" layer="91" />
                <label x="256.54" y="119.38" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C17" gate="G$1" pin="1" />
                <wire x1="320.04" y1="127" x2="317.5" y2="127" width="0.1524" layer="91" />
                <label x="317.5" y="127" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="VIO" />
                <wire x1="167.64" y1="190.5" x2="165.1" y2="190.5" width="0.1524" layer="91" />
                <label x="165.1" y="190.5" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C23" gate="G$1" pin="1" />
                <wire x1="350.52" y1="172.72" x2="347.98" y2="172.72" width="0.1524" layer="91" />
                <label x="347.98" y="172.72" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R44" gate="G$1" pin="1" />
                <wire x1="325.12" y1="314.96" x2="322.58" y2="314.96" width="0.1524" layer="91" />
                <label x="322.58" y="314.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="VDD" />
                <wire x1="365.76" y1="33.02" x2="363.22" y2="33.02" width="0.1524" layer="91" />
                <label x="363.22" y="33.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C24" gate="G$1" pin="1" />
                <wire x1="457.2" y1="30.48" x2="454.66" y2="30.48" width="0.1524" layer="91" />
                <label x="454.66" y="30.48" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RTC_3V0" class="0">
              <segment>
                <pinref part="U6" gate="G$1" pin="VOUT" />
                <wire x1="279.4" y1="119.38" x2="281.94" y2="119.38" width="0.1524" layer="91" />
                <label x="281.94" y="119.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C18" gate="G$1" pin="1" />
                <wire x1="320.04" y1="109.22" x2="317.5" y2="109.22" width="0.1524" layer="91" />
                <label x="317.5" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D6" gate="G$1" pin="A" />
                <wire x1="152.4" y1="91.44" x2="149.86" y2="91.44" width="0.1524" layer="91" />
                <label x="149.86" y="91.44" size="1.778" layer="95" rot="R180" />
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
                <wire x1="121.92" y1="355.6" x2="124.46" y2="355.6" width="0.1524" layer="91" />
                <label x="124.46" y="355.6" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="2" />
                <wire x1="172.72" y1="355.6" x2="175.26" y2="355.6" width="0.1524" layer="91" />
                <label x="175.26" y="355.6" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="2" />
                <wire x1="325.12" y1="330.2" x2="327.66" y2="330.2" width="0.1524" layer="91" />
                <label x="327.66" y="330.2" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="2" />
                <wire x1="269.24" y1="317.5" x2="266.7" y2="317.5" width="0.1524" layer="91" />
                <label x="266.7" y="317.5" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="6" />
                <wire x1="269.24" y1="297.18" x2="266.7" y2="297.18" width="0.1524" layer="91" />
                <label x="266.7" y="297.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@1" />
                <wire x1="55.88" y1="292.1" x2="53.34" y2="292.1" width="0.1524" layer="91" />
                <label x="53.34" y="292.1" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@2" />
                <wire x1="55.88" y1="266.7" x2="53.34" y2="266.7" width="0.1524" layer="91" />
                <label x="53.34" y="266.7" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@3" />
                <wire x1="55.88" y1="241.3" x2="53.34" y2="241.3" width="0.1524" layer="91" />
                <label x="53.34" y="241.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@4" />
                <wire x1="55.88" y1="215.9" x2="53.34" y2="215.9" width="0.1524" layer="91" />
                <label x="53.34" y="215.9" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@5" />
                <wire x1="96.52" y1="215.9" x2="99.06" y2="215.9" width="0.1524" layer="91" />
                <label x="99.06" y="215.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@6" />
                <wire x1="96.52" y1="241.3" x2="99.06" y2="241.3" width="0.1524" layer="91" />
                <label x="99.06" y="241.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@7" />
                <wire x1="96.52" y1="266.7" x2="99.06" y2="266.7" width="0.1524" layer="91" />
                <label x="99.06" y="266.7" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@8" />
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
                <wire x1="162.56" y1="157.48" x2="165.1" y2="157.48" width="0.1524" layer="91" />
                <label x="165.1" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="2" />
                <wire x1="218.44" y1="157.48" x2="220.98" y2="157.48" width="0.1524" layer="91" />
                <label x="220.98" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="2" />
                <wire x1="274.32" y1="157.48" x2="276.86" y2="157.48" width="0.1524" layer="91" />
                <label x="276.86" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="2" />
                <wire x1="330.2" y1="157.48" x2="332.74" y2="157.48" width="0.1524" layer="91" />
                <label x="332.74" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="2" />
                <wire x1="162.56" y1="144.78" x2="165.1" y2="144.78" width="0.1524" layer="91" />
                <label x="165.1" y="144.78" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="2" />
                <wire x1="330.2" y1="144.78" x2="332.74" y2="144.78" width="0.1524" layer="91" />
                <label x="332.74" y="144.78" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP1" gate="G$1" pin="TP" />
                <wire x1="434.34" y1="381" x2="431.8" y2="381" width="0.1524" layer="91" />
                <label x="431.8" y="381" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="GND" />
                <wire x1="58.42" y1="93.98" x2="55.88" y2="93.98" width="0.1524" layer="91" />
                <label x="55.88" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@5" />
                <wire x1="93.98" y1="129.54" x2="96.52" y2="129.54" width="0.1524" layer="91" />
                <label x="96.52" y="129.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@6" />
                <wire x1="93.98" y1="124.46" x2="96.52" y2="124.46" width="0.1524" layer="91" />
                <label x="96.52" y="124.46" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@7" />
                <wire x1="93.98" y1="119.38" x2="96.52" y2="119.38" width="0.1524" layer="91" />
                <label x="96.52" y="119.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@8" />
                <wire x1="93.98" y1="114.3" x2="96.52" y2="114.3" width="0.1524" layer="91" />
                <label x="96.52" y="114.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@9" />
                <wire x1="93.98" y1="109.22" x2="96.52" y2="109.22" width="0.1524" layer="91" />
                <label x="96.52" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@10" />
                <wire x1="93.98" y1="104.14" x2="96.52" y2="104.14" width="0.1524" layer="91" />
                <label x="96.52" y="104.14" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@11" />
                <wire x1="93.98" y1="99.06" x2="96.52" y2="99.06" width="0.1524" layer="91" />
                <label x="96.52" y="99.06" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="NC@12" />
                <wire x1="93.98" y1="93.98" x2="96.52" y2="93.98" width="0.1524" layer="91" />
                <label x="96.52" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C14" gate="G$1" pin="2" />
                <wire x1="162.56" y1="127" x2="165.1" y2="127" width="0.1524" layer="91" />
                <label x="165.1" y="127" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="BT1" gate="G$1" pin="-" />
                <wire x1="162.56" y1="109.22" x2="165.1" y2="109.22" width="0.1524" layer="91" />
                <label x="165.1" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C15" gate="G$1" pin="2" />
                <wire x1="218.44" y1="109.22" x2="220.98" y2="109.22" width="0.1524" layer="91" />
                <label x="220.98" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J12" gate="G$1" pin="GND" />
                <wire x1="421.64" y1="294.64" x2="419.1" y2="294.64" width="0.1524" layer="91" />
                <label x="419.1" y="294.64" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C16" gate="G$1" pin="2" />
                <wire x1="436.88" y1="185.42" x2="439.42" y2="185.42" width="0.1524" layer="91" />
                <label x="439.42" y="185.42" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q1" gate="G$1" pin="S" />
                <wire x1="441.96" y1="238.76" x2="444.5" y2="238.76" width="0.1524" layer="91" />
                <label x="444.5" y="238.76" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R24" gate="G$1" pin="2" />
                <wire x1="436.88" y1="223.52" x2="439.42" y2="223.52" width="0.1524" layer="91" />
                <label x="439.42" y="223.52" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U6" gate="G$1" pin="GND" />
                <wire x1="259.08" y1="114.3" x2="256.54" y2="114.3" width="0.1524" layer="91" />
                <label x="256.54" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C17" gate="G$1" pin="2" />
                <wire x1="330.2" y1="127" x2="332.74" y2="127" width="0.1524" layer="91" />
                <label x="332.74" y="127" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C18" gate="G$1" pin="2" />
                <wire x1="330.2" y1="109.22" x2="332.74" y2="109.22" width="0.1524" layer="91" />
                <label x="332.74" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R40" gate="G$1" pin="2" />
                <wire x1="274.32" y1="355.6" x2="276.86" y2="355.6" width="0.1524" layer="91" />
                <label x="276.86" y="355.6" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R43" gate="G$1" pin="2" />
                <wire x1="325.12" y1="355.6" x2="327.66" y2="355.6" width="0.1524" layer="91" />
                <label x="327.66" y="355.6" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C19" gate="G$1" pin="2" />
                <wire x1="325.12" y1="342.9" x2="327.66" y2="342.9" width="0.1524" layer="91" />
                <label x="327.66" y="342.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U9" gate="G$1" pin="A" />
                <wire x1="208.28" y1="330.2" x2="205.74" y2="330.2" width="0.1524" layer="91" />
                <label x="205.74" y="330.2" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="GND" />
                <wire x1="167.64" y1="180.34" x2="165.1" y2="180.34" width="0.1524" layer="91" />
                <label x="165.1" y="180.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D7" gate="G$1" pin="GND" />
                <wire x1="264.16" y1="200.66" x2="266.7" y2="200.66" width="0.1524" layer="91" />
                <label x="266.7" y="200.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R45" gate="G$1" pin="2" />
                <wire x1="259.08" y1="172.72" x2="261.62" y2="172.72" width="0.1524" layer="91" />
                <label x="261.62" y="172.72" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C20" gate="G$1" pin="2" />
                <wire x1="309.88" y1="172.72" x2="312.42" y2="172.72" width="0.1524" layer="91" />
                <label x="312.42" y="172.72" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C23" gate="G$1" pin="2" />
                <wire x1="360.68" y1="172.72" x2="363.22" y2="172.72" width="0.1524" layer="91" />
                <label x="363.22" y="172.72" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J14" gate="G$1" pin="2" />
                <wire x1="345.44" y1="200.66" x2="342.9" y2="200.66" width="0.1524" layer="91" />
                <label x="342.9" y="200.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J14" gate="G$1" pin="6" />
                <wire x1="345.44" y1="180.34" x2="342.9" y2="180.34" width="0.1524" layer="91" />
                <label x="342.9" y="180.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J15" gate="G$1" pin="2" />
                <wire x1="25.4" y1="55.88" x2="22.86" y2="55.88" width="0.1524" layer="91" />
                <label x="22.86" y="55.88" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q2" gate="G$1" pin="S" />
                <wire x1="96.52" y1="53.34" x2="99.06" y2="53.34" width="0.1524" layer="91" />
                <label x="99.06" y="53.34" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D8" gate="G$1" pin="A1" />
                <wire x1="177.8" y1="58.42" x2="175.26" y2="58.42" width="0.1524" layer="91" />
                <label x="175.26" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C22" gate="G$1" pin="2" />
                <wire x1="243.84" y1="60.96" x2="246.38" y2="60.96" width="0.1524" layer="91" />
                <label x="246.38" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R47" gate="G$1" pin="2" />
                <wire x1="243.84" y1="48.26" x2="246.38" y2="48.26" width="0.1524" layer="91" />
                <label x="246.38" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R48" gate="G$1" pin="2" />
                <wire x1="142.24" y1="48.26" x2="144.78" y2="48.26" width="0.1524" layer="91" />
                <label x="144.78" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="1" />
                <wire x1="370.84" y1="139.7" x2="368.3" y2="139.7" width="0.1524" layer="91" />
                <label x="368.3" y="139.7" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q3" gate="G$1" pin="S" />
                <wire x1="391.16" y1="88.9" x2="393.7" y2="88.9" width="0.1524" layer="91" />
                <label x="393.7" y="88.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q4" gate="G$1" pin="S" />
                <wire x1="431.8" y1="88.9" x2="434.34" y2="88.9" width="0.1524" layer="91" />
                <label x="434.34" y="88.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q5" gate="G$1" pin="S" />
                <wire x1="472.44" y1="88.9" x2="474.98" y2="88.9" width="0.1524" layer="91" />
                <label x="474.98" y="88.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="Q6" gate="G$1" pin="S" />
                <wire x1="513.08" y1="88.9" x2="515.62" y2="88.9" width="0.1524" layer="91" />
                <label x="515.62" y="88.9" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R50" gate="G$1" pin="2" />
                <wire x1="386.08" y1="60.96" x2="388.62" y2="60.96" width="0.1524" layer="91" />
                <label x="388.62" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R51" gate="G$1" pin="2" />
                <wire x1="426.72" y1="60.96" x2="429.26" y2="60.96" width="0.1524" layer="91" />
                <label x="429.26" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R52" gate="G$1" pin="2" />
                <wire x1="467.36" y1="60.96" x2="469.9" y2="60.96" width="0.1524" layer="91" />
                <label x="469.9" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R53" gate="G$1" pin="2" />
                <wire x1="508" y1="60.96" x2="510.54" y2="60.96" width="0.1524" layer="91" />
                <label x="510.54" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="VSS" />
                <wire x1="365.76" y1="17.78" x2="363.22" y2="17.78" width="0.1524" layer="91" />
                <label x="363.22" y="17.78" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="A0" />
                <wire x1="396.24" y1="33.02" x2="398.78" y2="33.02" width="0.1524" layer="91" />
                <label x="398.78" y="33.02" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="P0A" />
                <wire x1="396.24" y1="27.94" x2="398.78" y2="27.94" width="0.1524" layer="91" />
                <label x="398.78" y="27.94" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C24" gate="G$1" pin="2" />
                <wire x1="467.36" y1="30.48" x2="469.9" y2="30.48" width="0.1524" layer="91" />
                <label x="469.9" y="30.48" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="ECU_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO21" />
                <wire x1="96.52" y1="271.78" x2="99.06" y2="271.78" width="0.1524" layer="91" />
                <label x="99.06" y="271.78" size="1.778" layer="95" />
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
                <wire x1="96.52" y1="261.62" x2="99.06" y2="261.62" width="0.1524" layer="91" />
                <label x="99.06" y="261.62" size="1.778" layer="95" />
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
                <wire x1="96.52" y1="256.54" x2="99.06" y2="256.54" width="0.1524" layer="91" />
                <label x="99.06" y="256.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="S" />
                <wire x1="167.64" y1="287.02" x2="165.1" y2="287.02" width="0.1524" layer="91" />
                <label x="165.1" y="287.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="1" />
                <wire x1="152.4" y1="144.78" x2="149.86" y2="144.78" width="0.1524" layer="91" />
                <label x="149.86" y="144.78" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GAUGE_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO32" />
                <wire x1="96.52" y1="231.14" x2="99.06" y2="231.14" width="0.1524" layer="91" />
                <label x="99.06" y="231.14" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="D" />
                <wire x1="167.64" y1="251.46" x2="165.1" y2="251.46" width="0.1524" layer="91" />
                <label x="165.1" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="2" />
                <wire x1="218.44" y1="144.78" x2="220.98" y2="144.78" width="0.1524" layer="91" />
                <label x="220.98" y="144.78" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="GAUGE_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO33" />
                <wire x1="96.52" y1="226.06" x2="99.06" y2="226.06" width="0.1524" layer="91" />
                <label x="99.06" y="226.06" size="1.778" layer="95" />
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
                <wire x1="96.52" y1="276.86" x2="99.06" y2="276.86" width="0.1524" layer="91" />
                <label x="99.06" y="276.86" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R8" gate="G$1" pin="2" />
                <wire x1="274.32" y1="144.78" x2="276.86" y2="144.78" width="0.1524" layer="91" />
                <label x="276.86" y="144.78" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="1" />
                <wire x1="320.04" y1="144.78" x2="317.5" y2="144.78" width="0.1524" layer="91" />
                <label x="317.5" y="144.78" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="1" />
                <wire x1="320.04" y1="157.48" x2="317.5" y2="157.48" width="0.1524" layer="91" />
                <label x="317.5" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP4" gate="G$1" pin="TP" />
                <wire x1="434.34" y1="350.52" x2="431.8" y2="350.52" width="0.1524" layer="91" />
                <label x="431.8" y="350.52" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RST" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="RUN" />
                <wire x1="96.52" y1="251.46" x2="99.06" y2="251.46" width="0.1524" layer="91" />
                <label x="99.06" y="251.46" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP5" gate="G$1" pin="TP" />
                <wire x1="434.34" y1="340.36" x2="431.8" y2="340.36" width="0.1524" layer="91" />
                <label x="431.8" y="340.36" size="1.778" layer="95" rot="R180" />
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
                <wire x1="269.24" y1="312.42" x2="266.7" y2="312.42" width="0.1524" layer="91" />
                <label x="266.7" y="312.42" size="1.778" layer="95" rot="R180" />
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
                <wire x1="269.24" y1="307.34" x2="266.7" y2="307.34" width="0.1524" layer="91" />
                <label x="266.7" y="307.34" size="1.778" layer="95" rot="R180" />
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
            <net name="CHAIN_SENSE" class="0">
              <segment>
                <pinref part="J11" gate="G$1" pin="5" />
                <wire x1="269.24" y1="302.26" x2="266.7" y2="302.26" width="0.1524" layer="91" />
                <label x="266.7" y="302.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R44" gate="G$1" pin="2" />
                <wire x1="335.28" y1="314.96" x2="337.82" y2="314.96" width="0.1524" layer="91" />
                <label x="337.82" y="314.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO51" />
                <wire x1="55.88" y1="297.18" x2="53.34" y2="297.18" width="0.1524" layer="91" />
                <label x="53.34" y="297.18" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="FG_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO26" />
                <wire x1="96.52" y1="246.38" x2="99.06" y2="246.38" width="0.1524" layer="91" />
                <label x="99.06" y="246.38" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="TXD" />
                <wire x1="167.64" y1="205.74" x2="165.1" y2="205.74" width="0.1524" layer="91" />
                <label x="165.1" y="205.74" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="FG_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO27" />
                <wire x1="96.52" y1="236.22" x2="99.06" y2="236.22" width="0.1524" layer="91" />
                <label x="99.06" y="236.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="RXD" />
                <wire x1="167.64" y1="200.66" x2="165.1" y2="200.66" width="0.1524" layer="91" />
                <label x="165.1" y="200.66" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="FG_SILENT" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO52" />
                <wire x1="55.88" y1="302.26" x2="53.34" y2="302.26" width="0.1524" layer="91" />
                <label x="53.34" y="302.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="S" />
                <wire x1="167.64" y1="195.58" x2="165.1" y2="195.58" width="0.1524" layer="91" />
                <label x="165.1" y="195.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R45" gate="G$1" pin="1" />
                <wire x1="248.92" y1="172.72" x2="246.38" y2="172.72" width="0.1524" layer="91" />
                <label x="246.38" y="172.72" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="FG_CANH" class="0">
              <segment>
                <pinref part="J14" gate="G$1" pin="3" />
                <wire x1="345.44" y1="195.58" x2="342.9" y2="195.58" width="0.1524" layer="91" />
                <label x="342.9" y="195.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="CANH" />
                <wire x1="198.12" y1="205.74" x2="200.66" y2="205.74" width="0.1524" layer="91" />
                <label x="200.66" y="205.74" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D7" gate="G$1" pin="IO1" />
                <wire x1="243.84" y1="200.66" x2="241.3" y2="200.66" width="0.1524" layer="91" />
                <label x="241.3" y="200.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R46" gate="G$1" pin="1" />
                <wire x1="248.92" y1="185.42" x2="246.38" y2="185.42" width="0.1524" layer="91" />
                <label x="246.38" y="185.42" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="FG_CANL" class="0">
              <segment>
                <pinref part="J14" gate="G$1" pin="4" />
                <wire x1="345.44" y1="190.5" x2="342.9" y2="190.5" width="0.1524" layer="91" />
                <label x="342.9" y="190.5" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U7" gate="G$1" pin="CANL" />
                <wire x1="198.12" y1="200.66" x2="200.66" y2="200.66" width="0.1524" layer="91" />
                <label x="200.66" y="200.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D7" gate="G$1" pin="IO2" />
                <wire x1="243.84" y1="195.58" x2="241.3" y2="195.58" width="0.1524" layer="91" />
                <label x="241.3" y="195.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP3" gate="G$1" pin="2" />
                <wire x1="309.88" y1="185.42" x2="312.42" y2="185.42" width="0.1524" layer="91" />
                <label x="312.42" y="185.42" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="FG_TERM" class="0">
              <segment>
                <pinref part="R46" gate="G$1" pin="2" />
                <wire x1="259.08" y1="185.42" x2="261.62" y2="185.42" width="0.1524" layer="91" />
                <label x="261.62" y="185.42" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP3" gate="G$1" pin="1" />
                <wire x1="299.72" y1="185.42" x2="297.18" y2="185.42" width="0.1524" layer="91" />
                <label x="297.18" y="185.42" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="HORN_DRV" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO49" />
                <wire x1="55.88" y1="256.54" x2="53.34" y2="256.54" width="0.1524" layer="91" />
                <label x="53.34" y="256.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C21" gate="G$1" pin="1" />
                <wire x1="132.08" y1="60.96" x2="129.54" y2="60.96" width="0.1524" layer="91" />
                <label x="129.54" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R48" gate="G$1" pin="1" />
                <wire x1="132.08" y1="48.26" x2="129.54" y2="48.26" width="0.1524" layer="91" />
                <label x="129.54" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R49" gate="G$1" pin="1" />
                <wire x1="284.48" y1="60.96" x2="281.94" y2="60.96" width="0.1524" layer="91" />
                <label x="281.94" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="HORN_PUMP" class="0">
              <segment>
                <pinref part="C21" gate="G$1" pin="2" />
                <wire x1="142.24" y1="60.96" x2="144.78" y2="60.96" width="0.1524" layer="91" />
                <label x="144.78" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D8" gate="G$1" pin="COM" />
                <wire x1="198.12" y1="58.42" x2="200.66" y2="58.42" width="0.1524" layer="91" />
                <label x="200.66" y="58.42" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="HORN_GATE" class="0">
              <segment>
                <pinref part="D8" gate="G$1" pin="K2" />
                <wire x1="177.8" y1="53.34" x2="175.26" y2="53.34" width="0.1524" layer="91" />
                <label x="175.26" y="53.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C22" gate="G$1" pin="1" />
                <wire x1="233.68" y1="60.96" x2="231.14" y2="60.96" width="0.1524" layer="91" />
                <label x="231.14" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R47" gate="G$1" pin="1" />
                <wire x1="233.68" y1="48.26" x2="231.14" y2="48.26" width="0.1524" layer="91" />
                <label x="231.14" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q2" gate="G$1" pin="G" />
                <wire x1="76.2" y1="58.42" x2="73.66" y2="58.42" width="0.1524" layer="91" />
                <label x="73.66" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP4" gate="G$1" pin="2" />
                <wire x1="294.64" y1="48.26" x2="297.18" y2="48.26" width="0.1524" layer="91" />
                <label x="297.18" y="48.26" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="HORN_DIRECT" class="0">
              <segment>
                <pinref part="R49" gate="G$1" pin="2" />
                <wire x1="294.64" y1="60.96" x2="297.18" y2="60.96" width="0.1524" layer="91" />
                <label x="297.18" y="60.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP4" gate="G$1" pin="1" />
                <wire x1="284.48" y1="48.26" x2="281.94" y2="48.26" width="0.1524" layer="91" />
                <label x="281.94" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="HORN_IN" class="0">
              <segment>
                <pinref part="J15" gate="G$1" pin="3" />
                <wire x1="25.4" y1="50.8" x2="22.86" y2="50.8" width="0.1524" layer="91" />
                <label x="22.86" y="50.8" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q2" gate="G$1" pin="D" />
                <wire x1="96.52" y1="58.42" x2="99.06" y2="58.42" width="0.1524" layer="91" />
                <label x="99.06" y="58.42" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="SWC_0" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO2" />
                <wire x1="55.88" y1="231.14" x2="53.34" y2="231.14" width="0.1524" layer="91" />
                <label x="53.34" y="231.14" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q3" gate="G$1" pin="G" />
                <wire x1="370.84" y1="93.98" x2="368.3" y2="93.98" width="0.1524" layer="91" />
                <label x="368.3" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R50" gate="G$1" pin="1" />
                <wire x1="375.92" y1="60.96" x2="373.38" y2="60.96" width="0.1524" layer="91" />
                <label x="373.38" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SW1" class="0">
              <segment>
                <pinref part="Q3" gate="G$1" pin="D" />
                <wire x1="391.16" y1="93.98" x2="393.7" y2="93.98" width="0.1524" layer="91" />
                <label x="393.7" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R30" gate="G$1" pin="1" />
                <wire x1="375.92" y1="73.66" x2="373.38" y2="73.66" width="0.1524" layer="91" />
                <label x="373.38" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="3" />
                <wire x1="370.84" y1="129.54" x2="368.3" y2="129.54" width="0.1524" layer="91" />
                <label x="368.3" y="129.54" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SWC_1" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO3" />
                <wire x1="55.88" y1="236.22" x2="53.34" y2="236.22" width="0.1524" layer="91" />
                <label x="53.34" y="236.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q4" gate="G$1" pin="G" />
                <wire x1="411.48" y1="93.98" x2="408.94" y2="93.98" width="0.1524" layer="91" />
                <label x="408.94" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R51" gate="G$1" pin="1" />
                <wire x1="416.56" y1="60.96" x2="414.02" y2="60.96" width="0.1524" layer="91" />
                <label x="414.02" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SW2" class="0">
              <segment>
                <pinref part="Q4" gate="G$1" pin="D" />
                <wire x1="431.8" y1="93.98" x2="434.34" y2="93.98" width="0.1524" layer="91" />
                <label x="434.34" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R31" gate="G$1" pin="1" />
                <wire x1="416.56" y1="73.66" x2="414.02" y2="73.66" width="0.1524" layer="91" />
                <label x="414.02" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="4" />
                <wire x1="370.84" y1="124.46" x2="368.3" y2="124.46" width="0.1524" layer="91" />
                <label x="368.3" y="124.46" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SWC_2" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO4" />
                <wire x1="55.88" y1="246.38" x2="53.34" y2="246.38" width="0.1524" layer="91" />
                <label x="53.34" y="246.38" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q5" gate="G$1" pin="G" />
                <wire x1="452.12" y1="93.98" x2="449.58" y2="93.98" width="0.1524" layer="91" />
                <label x="449.58" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R52" gate="G$1" pin="1" />
                <wire x1="457.2" y1="60.96" x2="454.66" y2="60.96" width="0.1524" layer="91" />
                <label x="454.66" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SW3" class="0">
              <segment>
                <pinref part="Q5" gate="G$1" pin="D" />
                <wire x1="472.44" y1="93.98" x2="474.98" y2="93.98" width="0.1524" layer="91" />
                <label x="474.98" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R32" gate="G$1" pin="1" />
                <wire x1="457.2" y1="73.66" x2="454.66" y2="73.66" width="0.1524" layer="91" />
                <label x="454.66" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="5" />
                <wire x1="370.84" y1="119.38" x2="368.3" y2="119.38" width="0.1524" layer="91" />
                <label x="368.3" y="119.38" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SWC_3" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO5" />
                <wire x1="55.88" y1="251.46" x2="53.34" y2="251.46" width="0.1524" layer="91" />
                <label x="53.34" y="251.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q6" gate="G$1" pin="G" />
                <wire x1="492.76" y1="93.98" x2="490.22" y2="93.98" width="0.1524" layer="91" />
                <label x="490.22" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R53" gate="G$1" pin="1" />
                <wire x1="497.84" y1="60.96" x2="495.3" y2="60.96" width="0.1524" layer="91" />
                <label x="495.3" y="60.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SW4" class="0">
              <segment>
                <pinref part="Q6" gate="G$1" pin="D" />
                <wire x1="513.08" y1="93.98" x2="515.62" y2="93.98" width="0.1524" layer="91" />
                <label x="515.62" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R33" gate="G$1" pin="1" />
                <wire x1="497.84" y1="73.66" x2="495.3" y2="73.66" width="0.1524" layer="91" />
                <label x="495.3" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="6" />
                <wire x1="370.84" y1="114.3" x2="368.3" y2="114.3" width="0.1524" layer="91" />
                <label x="368.3" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="REM" class="0">
              <segment>
                <pinref part="R30" gate="G$1" pin="2" />
                <wire x1="386.08" y1="73.66" x2="388.62" y2="73.66" width="0.1524" layer="91" />
                <label x="388.62" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R31" gate="G$1" pin="2" />
                <wire x1="426.72" y1="73.66" x2="429.26" y2="73.66" width="0.1524" layer="91" />
                <label x="429.26" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R32" gate="G$1" pin="2" />
                <wire x1="467.36" y1="73.66" x2="469.9" y2="73.66" width="0.1524" layer="91" />
                <label x="469.9" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R33" gate="G$1" pin="2" />
                <wire x1="508" y1="73.66" x2="510.54" y2="73.66" width="0.1524" layer="91" />
                <label x="510.54" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J13" gate="G$1" pin="2" />
                <wire x1="370.84" y1="134.62" x2="368.3" y2="134.62" width="0.1524" layer="91" />
                <label x="368.3" y="134.62" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="P0W" />
                <wire x1="396.24" y1="22.86" x2="398.78" y2="22.86" width="0.1524" layer="91" />
                <label x="398.78" y="22.86" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="I2C_SDA" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO7_SDA" />
                <wire x1="55.88" y1="220.98" x2="53.34" y2="220.98" width="0.1524" layer="91" />
                <label x="53.34" y="220.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="SDA" />
                <wire x1="58.42" y1="119.38" x2="55.88" y2="119.38" width="0.1524" layer="91" />
                <label x="55.88" y="119.38" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="SDA" />
                <wire x1="365.76" y1="22.86" x2="363.22" y2="22.86" width="0.1524" layer="91" />
                <label x="363.22" y="22.86" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="I2C_SCL" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO8_SCL" />
                <wire x1="55.88" y1="226.06" x2="53.34" y2="226.06" width="0.1524" layer="91" />
                <label x="53.34" y="226.06" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="SCL" />
                <wire x1="58.42" y1="114.3" x2="55.88" y2="114.3" width="0.1524" layer="91" />
                <label x="55.88" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U8" gate="G$1" pin="SCL" />
                <wire x1="365.76" y1="27.94" x2="363.22" y2="27.94" width="0.1524" layer="91" />
                <label x="363.22" y="27.94" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RTC_INT" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO31" />
                <wire x1="55.88" y1="287.02" x2="53.34" y2="287.02" width="0.1524" layer="91" />
                <label x="53.34" y="287.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U5" gate="G$1" pin="INT_SQW" />
                <wire x1="58.42" y1="109.22" x2="55.88" y2="109.22" width="0.1524" layer="91" />
                <label x="55.88" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R20" gate="G$1" pin="2" />
                <wire x1="218.44" y1="127" x2="220.98" y2="127" width="0.1524" layer="91" />
                <label x="220.98" y="127" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="RTC_VBAT" class="0">
              <segment>
                <pinref part="U5" gate="G$1" pin="VBAT" />
                <wire x1="58.42" y1="124.46" x2="55.88" y2="124.46" width="0.1524" layer="91" />
                <label x="55.88" y="124.46" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="BT1" gate="G$1" pin="+" />
                <wire x1="152.4" y1="109.22" x2="149.86" y2="109.22" width="0.1524" layer="91" />
                <label x="149.86" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C15" gate="G$1" pin="1" />
                <wire x1="208.28" y1="109.22" x2="205.74" y2="109.22" width="0.1524" layer="91" />
                <label x="205.74" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP2" gate="G$1" pin="2" />
                <wire x1="274.32" y1="91.44" x2="276.86" y2="91.44" width="0.1524" layer="91" />
                <label x="276.86" y="91.44" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="RTC_CHG_A" class="0">
              <segment>
                <pinref part="D6" gate="G$1" pin="K" />
                <wire x1="162.56" y1="91.44" x2="165.1" y2="91.44" width="0.1524" layer="91" />
                <label x="165.1" y="91.44" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R21" gate="G$1" pin="1" />
                <wire x1="208.28" y1="91.44" x2="205.74" y2="91.44" width="0.1524" layer="91" />
                <label x="205.74" y="91.44" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RTC_CHG_B" class="0">
              <segment>
                <pinref part="R21" gate="G$1" pin="2" />
                <wire x1="218.44" y1="91.44" x2="220.98" y2="91.44" width="0.1524" layer="91" />
                <label x="220.98" y="91.44" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP2" gate="G$1" pin="1" />
                <wire x1="264.16" y1="91.44" x2="261.62" y2="91.44" width="0.1524" layer="91" />
                <label x="261.62" y="91.44" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_TX" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="TX" />
                <wire x1="421.64" y1="284.48" x2="419.1" y2="284.48" width="0.1524" layer="91" />
                <label x="419.1" y="284.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R22" gate="G$1" pin="1" />
                <wire x1="426.72" y1="210.82" x2="424.18" y2="210.82" width="0.1524" layer="91" />
                <label x="424.18" y="210.82" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_TX_IN" class="0">
              <segment>
                <pinref part="R22" gate="G$1" pin="2" />
                <wire x1="436.88" y1="210.82" x2="439.42" y2="210.82" width="0.1524" layer="91" />
                <label x="439.42" y="210.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO28" />
                <wire x1="55.88" y1="271.78" x2="53.34" y2="271.78" width="0.1524" layer="91" />
                <label x="53.34" y="271.78" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_RX" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="RX" />
                <wire x1="421.64" y1="289.56" x2="419.1" y2="289.56" width="0.1524" layer="91" />
                <label x="419.1" y="289.56" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO29" />
                <wire x1="55.88" y1="276.86" x2="53.34" y2="276.86" width="0.1524" layer="91" />
                <label x="53.34" y="276.86" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_PPS" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="PPS" />
                <wire x1="421.64" y1="304.8" x2="419.1" y2="304.8" width="0.1524" layer="91" />
                <label x="419.1" y="304.8" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R23" gate="G$1" pin="1" />
                <wire x1="426.72" y1="198.12" x2="424.18" y2="198.12" width="0.1524" layer="91" />
                <label x="424.18" y="198.12" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_PPS_IN" class="0">
              <segment>
                <pinref part="R23" gate="G$1" pin="2" />
                <wire x1="436.88" y1="198.12" x2="439.42" y2="198.12" width="0.1524" layer="91" />
                <label x="439.42" y="198.12" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="IO30" />
                <wire x1="55.88" y1="281.94" x2="53.34" y2="281.94" width="0.1524" layer="91" />
                <label x="53.34" y="281.94" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GPS_EN" class="0">
              <segment>
                <pinref part="J12" gate="G$1" pin="EN" />
                <wire x1="421.64" y1="269.24" x2="419.1" y2="269.24" width="0.1524" layer="91" />
                <label x="419.1" y="269.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q1" gate="G$1" pin="D" />
                <wire x1="441.96" y1="243.84" x2="444.5" y2="243.84" width="0.1524" layer="91" />
                <label x="444.5" y="243.84" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="GPS_OFF" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO50" />
                <wire x1="55.88" y1="261.62" x2="53.34" y2="261.62" width="0.1524" layer="91" />
                <label x="53.34" y="261.62" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="Q1" gate="G$1" pin="G" />
                <wire x1="421.64" y1="243.84" x2="419.1" y2="243.84" width="0.1524" layer="91" />
                <label x="419.1" y="243.84" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R24" gate="G$1" pin="1" />
                <wire x1="426.72" y1="223.52" x2="424.18" y2="223.52" width="0.1524" layer="91" />
                <label x="424.18" y="223.52" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
          </nets>
        </sheet>
      </sheets>
    </schematic>
  </drawing>
</eagle>
