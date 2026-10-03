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
            <package name="ESP32-C6-DEVKITC-1">
              <description>Top view, USB ports at the bottom. Board outline 25.4 x 51.8 mm; the antenna overhangs the top edge by about 6.35 mm: keep copper clear of it.</description>
              <pad name="J1_1" x="-11.43" y="19.05" drill="1" diameter="1.8" shape="square" />
              <pad name="J3_1" x="11.43" y="19.05" drill="1" diameter="1.8" shape="square" />
              <pad name="J1_2" x="-11.43" y="16.51" drill="1" diameter="1.8" />
              <pad name="J3_2" x="11.43" y="16.51" drill="1" diameter="1.8" />
              <pad name="J1_3" x="-11.43" y="13.97" drill="1" diameter="1.8" />
              <pad name="J3_3" x="11.43" y="13.97" drill="1" diameter="1.8" />
              <pad name="J1_4" x="-11.43" y="11.43" drill="1" diameter="1.8" />
              <pad name="J3_4" x="11.43" y="11.43" drill="1" diameter="1.8" />
              <pad name="J1_5" x="-11.43" y="8.89" drill="1" diameter="1.8" />
              <pad name="J3_5" x="11.43" y="8.89" drill="1" diameter="1.8" />
              <pad name="J1_6" x="-11.43" y="6.35" drill="1" diameter="1.8" />
              <pad name="J3_6" x="11.43" y="6.35" drill="1" diameter="1.8" />
              <pad name="J1_7" x="-11.43" y="3.81" drill="1" diameter="1.8" />
              <pad name="J3_7" x="11.43" y="3.81" drill="1" diameter="1.8" />
              <pad name="J1_8" x="-11.43" y="1.27" drill="1" diameter="1.8" />
              <pad name="J3_8" x="11.43" y="1.27" drill="1" diameter="1.8" />
              <pad name="J1_9" x="-11.43" y="-1.27" drill="1" diameter="1.8" />
              <pad name="J3_9" x="11.43" y="-1.27" drill="1" diameter="1.8" />
              <pad name="J1_10" x="-11.43" y="-3.81" drill="1" diameter="1.8" />
              <pad name="J3_10" x="11.43" y="-3.81" drill="1" diameter="1.8" />
              <pad name="J1_11" x="-11.43" y="-6.35" drill="1" diameter="1.8" />
              <pad name="J3_11" x="11.43" y="-6.35" drill="1" diameter="1.8" />
              <pad name="J1_12" x="-11.43" y="-8.89" drill="1" diameter="1.8" />
              <pad name="J3_12" x="11.43" y="-8.89" drill="1" diameter="1.8" />
              <pad name="J1_13" x="-11.43" y="-11.43" drill="1" diameter="1.8" />
              <pad name="J3_13" x="11.43" y="-11.43" drill="1" diameter="1.8" />
              <pad name="J1_14" x="-11.43" y="-13.97" drill="1" diameter="1.8" />
              <pad name="J3_14" x="11.43" y="-13.97" drill="1" diameter="1.8" />
              <pad name="J1_15" x="-11.43" y="-16.51" drill="1" diameter="1.8" />
              <pad name="J3_15" x="11.43" y="-16.51" drill="1" diameter="1.8" />
              <pad name="J1_16" x="-11.43" y="-19.05" drill="1" diameter="1.8" />
              <pad name="J3_16" x="11.43" y="-19.05" drill="1" diameter="1.8" />
              <wire x1="-12.7" y1="20.625" x2="12.7" y2="20.625" width="0.127" layer="21" />
              <wire x1="12.7" y1="20.625" x2="12.7" y2="-31.175" width="0.127" layer="21" />
              <wire x1="12.7" y1="-31.175" x2="-12.7" y2="-31.175" width="0.127" layer="21" />
              <wire x1="-12.7" y1="-31.175" x2="-12.7" y2="20.625" width="0.127" layer="21" />
              <text x="-12.7" y="21.2" size="1.016" layer="25">&gt;NAME</text>
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
            <symbol name="ESP32-C6-DEVKITC-1">
              <wire x1="-15.24" y1="41.91" x2="15.24" y2="41.91" width="0.254" layer="94" />
              <wire x1="15.24" y1="41.91" x2="15.24" y2="-41.91" width="0.254" layer="94" />
              <wire x1="15.24" y1="-41.91" x2="-15.24" y2="-41.91" width="0.254" layer="94" />
              <wire x1="-15.24" y1="-41.91" x2="-15.24" y2="41.91" width="0.254" layer="94" />
              <pin name="3V3" x="-20.32" y="38.1" visible="pin" length="middle" direction="pas" />
              <pin name="RST" x="-20.32" y="33.02" visible="pin" length="middle" direction="pas" />
              <pin name="IO4" x="-20.32" y="27.94" visible="pin" length="middle" direction="pas" />
              <pin name="IO5" x="-20.32" y="22.86" visible="pin" length="middle" direction="pas" />
              <pin name="IO6" x="-20.32" y="17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO7" x="-20.32" y="12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO0" x="-20.32" y="7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO1" x="-20.32" y="2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO8" x="-20.32" y="-2.54" visible="pin" length="middle" direction="pas" />
              <pin name="IO10" x="-20.32" y="-7.62" visible="pin" length="middle" direction="pas" />
              <pin name="IO11" x="-20.32" y="-12.7" visible="pin" length="middle" direction="pas" />
              <pin name="IO2" x="-20.32" y="-17.78" visible="pin" length="middle" direction="pas" />
              <pin name="IO3" x="-20.32" y="-22.86" visible="pin" length="middle" direction="pas" />
              <pin name="5V" x="-20.32" y="-27.94" visible="pin" length="middle" direction="pas" />
              <pin name="GND@1" x="-20.32" y="-33.02" visible="pin" length="middle" direction="pas" />
              <pin name="NC@1" x="-20.32" y="-38.1" visible="pin" length="middle" direction="pas" />
              <pin name="GND@2" x="20.32" y="38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO16_TX" x="20.32" y="33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO17_RX" x="20.32" y="27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO15" x="20.32" y="22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO23" x="20.32" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO22" x="20.32" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO21" x="20.32" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO20" x="20.32" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO19" x="20.32" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO18" x="20.32" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO9" x="20.32" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@3" x="20.32" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO13_USB_DP" x="20.32" y="-22.86" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="IO12_USB_DM" x="20.32" y="-27.94" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND@4" x="20.32" y="-33.02" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="NC@2" x="20.32" y="-38.1" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-15.24" y="42.91" size="1.778" layer="95">&gt;NAME</text>
              <text x="-15.24" y="-44.71" size="1.778" layer="96">&gt;VALUE</text>
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
            <deviceset name="ESP32-C6-DEVKITC-1" prefix="U" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="ESP32-C6-DEVKITC-1" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="ESP32-C6-DEVKITC-1">
                  <connects>
                    <connect gate="G$1" pin="3V3" pad="J1_1" />
                    <connect gate="G$1" pin="RST" pad="J1_2" />
                    <connect gate="G$1" pin="IO4" pad="J1_3" />
                    <connect gate="G$1" pin="IO5" pad="J1_4" />
                    <connect gate="G$1" pin="IO6" pad="J1_5" />
                    <connect gate="G$1" pin="IO7" pad="J1_6" />
                    <connect gate="G$1" pin="IO0" pad="J1_7" />
                    <connect gate="G$1" pin="IO1" pad="J1_8" />
                    <connect gate="G$1" pin="IO8" pad="J1_9" />
                    <connect gate="G$1" pin="IO10" pad="J1_10" />
                    <connect gate="G$1" pin="IO11" pad="J1_11" />
                    <connect gate="G$1" pin="IO2" pad="J1_12" />
                    <connect gate="G$1" pin="IO3" pad="J1_13" />
                    <connect gate="G$1" pin="5V" pad="J1_14" />
                    <connect gate="G$1" pin="GND@1" pad="J1_15" />
                    <connect gate="G$1" pin="NC@1" pad="J1_16" />
                    <connect gate="G$1" pin="GND@2" pad="J3_1" />
                    <connect gate="G$1" pin="IO16_TX" pad="J3_2" />
                    <connect gate="G$1" pin="IO17_RX" pad="J3_3" />
                    <connect gate="G$1" pin="IO15" pad="J3_4" />
                    <connect gate="G$1" pin="IO23" pad="J3_5" />
                    <connect gate="G$1" pin="IO22" pad="J3_6" />
                    <connect gate="G$1" pin="IO21" pad="J3_7" />
                    <connect gate="G$1" pin="IO20" pad="J3_8" />
                    <connect gate="G$1" pin="IO19" pad="J3_9" />
                    <connect gate="G$1" pin="IO18" pad="J3_10" />
                    <connect gate="G$1" pin="IO9" pad="J3_11" />
                    <connect gate="G$1" pin="GND@3" pad="J3_12" />
                    <connect gate="G$1" pin="IO13_USB_DP" pad="J3_13" />
                    <connect gate="G$1" pin="IO12_USB_DM" pad="J3_14" />
                    <connect gate="G$1" pin="GND@4" pad="J3_15" />
                    <connect gate="G$1" pin="NC@2" pad="J3_16" />
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
        <part name="D3" library="haltech-gauges" deviceset="SCHOTTKY-SMA" device="" value="SS14">
          <attribute name="LCSC" value="C2480" />
        </part>
        <part name="F2" library="haltech-gauges" deviceset="POLYFUSE-1812" device="" value="1812L200/16DR">
          <attribute name="LCSC" value="C439873" />
        </part>
        <part name="J11" library="haltech-gauges" deviceset="MICROFIT-2X2" device="" value="43045-0412 CHAIN" />
        <part name="U4" library="haltech-gauges" deviceset="ESP32-C6-DEVKITC-1" device="" value="ESP32-C6-DevKitC-1-N8" />
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
        <part name="TP1" library="haltech-gauges" deviceset="TESTPOINT" device="" value="GND" />
        <part name="TP2" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+5V" />
        <part name="TP3" library="haltech-gauges" deviceset="TESTPOINT" device="" value="+3V3" />
        <part name="TP4" library="haltech-gauges" deviceset="TESTPOINT" device="" value="VBAT" />
        <part name="TP5" library="haltech-gauges" deviceset="TESTPOINT" device="" value="RST" />
      </parts>
      <sheets>
        <sheet>
          <plain>
            <text x="10.16" y="239.44" size="2.54" layer="97">Haltech gauges: hub carrier</text>
            <text x="10.16" y="233.44" size="1.778" layer="97">J10 is the ECU side: 1 +12V switched, 2 GND, 3 CAN H, 4 CAN L (from the DTM-4 loom,</text>
            <text x="10.16" y="230.44" size="1.778" layer="97">which carries the inline 2 A fuse). J11 is the gauge chain: 1 +5V, 2 GND, 3 CAN H, 4 CAN L.</text>
            <text x="10.16" y="227.44" size="1.778" layer="97">U4 is the ESP32-C6-DevKitC-1-N8, soldered in. U1 is the Pololu D36V28F5.</text>
            <text x="10.16" y="224.44" size="1.778" layer="97">JP1 fitted = 120R across the ECU bus (leave open if the Haltech bus is already terminated).</text>
            <text x="10.16" y="221.44" size="1.778" layer="97">R4-R7 and C9 are the gauge bus's split termination (two 120R in parallel per leg).</text>
            <text x="10.16" y="218.44" size="1.778" layer="97">Footprints are generic; the Pololu one is a placeholder.</text>
          </plain>
          <instances>
            <instance part="J10" gate="G$1" x="35.56" y="187.96" />
            <instance part="D1" gate="G$1" x="86.36" y="187.96" />
            <instance part="D2" gate="G$1" x="137.16" y="187.96" />
            <instance part="C1" gate="G$1" x="187.96" y="187.96" />
            <instance part="C2" gate="G$1" x="238.76" y="187.96" />
            <instance part="U1" gate="G$1" x="45.72" y="152.4" />
            <instance part="C3" gate="G$1" x="116.84" y="157.48" />
            <instance part="C4" gate="G$1" x="167.64" y="157.48" />
            <instance part="C5" gate="G$1" x="218.44" y="157.48" />
            <instance part="D3" gate="G$1" x="116.84" y="142.24" />
            <instance part="F2" gate="G$1" x="167.64" y="142.24" />
            <instance part="J11" gate="G$1" x="279.4" y="152.4" />
            <instance part="U4" gate="G$1" x="60.96" y="76.2" />
            <instance part="U2" gate="G$1" x="147.32" y="101.6" />
            <instance part="U3" gate="G$1" x="147.32" y="55.88" />
            <instance part="D4" gate="G$1" x="218.44" y="111.76" />
            <instance part="R3" gate="G$1" x="218.44" y="93.98" />
            <instance part="JP1" gate="G$1" x="269.24" y="93.98" />
            <instance part="D5" gate="G$1" x="218.44" y="71.12" />
            <instance part="R4" gate="G$1" x="218.44" y="53.34" />
            <instance part="R5" gate="G$1" x="269.24" y="53.34" />
            <instance part="R6" gate="G$1" x="218.44" y="40.64" />
            <instance part="R7" gate="G$1" x="269.24" y="40.64" />
            <instance part="C9" gate="G$1" x="218.44" y="27.94" />
            <instance part="C6" gate="G$1" x="30.48" y="20.32" />
            <instance part="C7" gate="G$1" x="86.36" y="20.32" />
            <instance part="C8" gate="G$1" x="142.24" y="20.32" />
            <instance part="C10" gate="G$1" x="198.12" y="15.24" />
            <instance part="R1" gate="G$1" x="30.48" y="7.62" />
            <instance part="R2" gate="G$1" x="86.36" y="7.62" />
            <instance part="R8" gate="G$1" x="142.24" y="7.62" />
            <instance part="R9" gate="G$1" x="198.12" y="2.54" />
            <instance part="TP1" gate="G$1" x="335.28" y="187.96" />
            <instance part="TP2" gate="G$1" x="335.28" y="177.8" />
            <instance part="TP3" gate="G$1" x="335.28" y="167.64" />
            <instance part="TP4" gate="G$1" x="335.28" y="157.48" />
            <instance part="TP5" gate="G$1" x="335.28" y="147.32" />
          </instances>
          <busses />
          <nets>
            <net name="+12V_IN" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="1" />
                <wire x1="25.4" y1="195.58" x2="22.86" y2="195.58" width="0.1524" layer="91" />
                <label x="22.86" y="195.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D1" gate="G$1" pin="1" />
                <wire x1="81.28" y1="187.96" x2="78.74" y2="187.96" width="0.1524" layer="91" />
                <label x="78.74" y="187.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D2" gate="G$1" pin="A" />
                <wire x1="132.08" y1="187.96" x2="129.54" y2="187.96" width="0.1524" layer="91" />
                <label x="129.54" y="187.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R8" gate="G$1" pin="1" />
                <wire x1="137.16" y1="7.62" x2="134.62" y2="7.62" width="0.1524" layer="91" />
                <label x="134.62" y="7.62" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+12V_PROT" class="0">
              <segment>
                <pinref part="D2" gate="G$1" pin="K" />
                <wire x1="142.24" y1="187.96" x2="144.78" y2="187.96" width="0.1524" layer="91" />
                <label x="144.78" y="187.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="+" />
                <wire x1="182.88" y1="187.96" x2="180.34" y2="187.96" width="0.1524" layer="91" />
                <label x="180.34" y="187.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C2" gate="G$1" pin="1" />
                <wire x1="233.68" y1="187.96" x2="231.14" y2="187.96" width="0.1524" layer="91" />
                <label x="231.14" y="187.96" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="VIN" />
                <wire x1="30.48" y1="157.48" x2="27.94" y2="157.48" width="0.1524" layer="91" />
                <label x="27.94" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V" class="0">
              <segment>
                <pinref part="U1" gate="G$1" pin="VOUT" />
                <wire x1="60.96" y1="157.48" x2="63.5" y2="157.48" width="0.1524" layer="91" />
                <label x="63.5" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C3" gate="G$1" pin="+" />
                <wire x1="111.76" y1="157.48" x2="109.22" y2="157.48" width="0.1524" layer="91" />
                <label x="109.22" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="1" />
                <wire x1="162.56" y1="157.48" x2="160.02" y2="157.48" width="0.1524" layer="91" />
                <label x="160.02" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="1" />
                <wire x1="213.36" y1="157.48" x2="210.82" y2="157.48" width="0.1524" layer="91" />
                <label x="210.82" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D3" gate="G$1" pin="A" />
                <wire x1="111.76" y1="142.24" x2="109.22" y2="142.24" width="0.1524" layer="91" />
                <label x="109.22" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="F2" gate="G$1" pin="1" />
                <wire x1="162.56" y1="142.24" x2="160.02" y2="142.24" width="0.1524" layer="91" />
                <label x="160.02" y="142.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VCC" />
                <wire x1="132.08" y1="93.98" x2="129.54" y2="93.98" width="0.1524" layer="91" />
                <label x="129.54" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C6" gate="G$1" pin="1" />
                <wire x1="25.4" y1="20.32" x2="22.86" y2="20.32" width="0.1524" layer="91" />
                <label x="22.86" y="20.32" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP2" gate="G$1" pin="TP" />
                <wire x1="332.74" y1="177.8" x2="330.2" y2="177.8" width="0.1524" layer="91" />
                <label x="330.2" y="177.8" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V_MCU" class="0">
              <segment>
                <pinref part="D3" gate="G$1" pin="K" />
                <wire x1="121.92" y1="142.24" x2="124.46" y2="142.24" width="0.1524" layer="91" />
                <label x="124.46" y="142.24" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="5V" />
                <wire x1="40.64" y1="48.26" x2="38.1" y2="48.26" width="0.1524" layer="91" />
                <label x="38.1" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+5V_CHAIN" class="0">
              <segment>
                <pinref part="F2" gate="G$1" pin="2" />
                <wire x1="172.72" y1="142.24" x2="175.26" y2="142.24" width="0.1524" layer="91" />
                <label x="175.26" y="142.24" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="1" />
                <wire x1="269.24" y1="160.02" x2="266.7" y2="160.02" width="0.1524" layer="91" />
                <label x="266.7" y="160.02" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="+3V3" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="3V3" />
                <wire x1="40.64" y1="114.3" x2="38.1" y2="114.3" width="0.1524" layer="91" />
                <label x="38.1" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="VIO" />
                <wire x1="132.08" y1="99.06" x2="129.54" y2="99.06" width="0.1524" layer="91" />
                <label x="129.54" y="99.06" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="1" />
                <wire x1="81.28" y1="20.32" x2="78.74" y2="20.32" width="0.1524" layer="91" />
                <label x="78.74" y="20.32" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="VCC" />
                <wire x1="132.08" y1="48.26" x2="129.54" y2="48.26" width="0.1524" layer="91" />
                <label x="129.54" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="1" />
                <wire x1="137.16" y1="20.32" x2="134.62" y2="20.32" width="0.1524" layer="91" />
                <label x="134.62" y="20.32" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="1" />
                <wire x1="81.28" y1="7.62" x2="78.74" y2="7.62" width="0.1524" layer="91" />
                <label x="78.74" y="7.62" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP3" gate="G$1" pin="TP" />
                <wire x1="332.74" y1="167.64" x2="330.2" y2="167.64" width="0.1524" layer="91" />
                <label x="330.2" y="167.64" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GND" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="2" />
                <wire x1="25.4" y1="190.5" x2="22.86" y2="190.5" width="0.1524" layer="91" />
                <label x="22.86" y="190.5" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D1" gate="G$1" pin="2" />
                <wire x1="91.44" y1="187.96" x2="93.98" y2="187.96" width="0.1524" layer="91" />
                <label x="93.98" y="187.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="-" />
                <wire x1="193.04" y1="187.96" x2="195.58" y2="187.96" width="0.1524" layer="91" />
                <label x="195.58" y="187.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C2" gate="G$1" pin="2" />
                <wire x1="243.84" y1="187.96" x2="246.38" y2="187.96" width="0.1524" layer="91" />
                <label x="246.38" y="187.96" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="GND@1" />
                <wire x1="30.48" y1="147.32" x2="27.94" y2="147.32" width="0.1524" layer="91" />
                <label x="27.94" y="147.32" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="GND@2" />
                <wire x1="60.96" y1="152.4" x2="63.5" y2="152.4" width="0.1524" layer="91" />
                <label x="63.5" y="152.4" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C3" gate="G$1" pin="-" />
                <wire x1="121.92" y1="157.48" x2="124.46" y2="157.48" width="0.1524" layer="91" />
                <label x="124.46" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C4" gate="G$1" pin="2" />
                <wire x1="172.72" y1="157.48" x2="175.26" y2="157.48" width="0.1524" layer="91" />
                <label x="175.26" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C5" gate="G$1" pin="2" />
                <wire x1="223.52" y1="157.48" x2="226.06" y2="157.48" width="0.1524" layer="91" />
                <label x="226.06" y="157.48" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="J11" gate="G$1" pin="2" />
                <wire x1="269.24" y1="154.94" x2="266.7" y2="154.94" width="0.1524" layer="91" />
                <label x="266.7" y="154.94" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@1" />
                <wire x1="40.64" y1="43.18" x2="38.1" y2="43.18" width="0.1524" layer="91" />
                <label x="38.1" y="43.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@2" />
                <wire x1="81.28" y1="114.3" x2="83.82" y2="114.3" width="0.1524" layer="91" />
                <label x="83.82" y="114.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@3" />
                <wire x1="81.28" y1="58.42" x2="83.82" y2="58.42" width="0.1524" layer="91" />
                <label x="83.82" y="58.42" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U4" gate="G$1" pin="GND@4" />
                <wire x1="81.28" y1="43.18" x2="83.82" y2="43.18" width="0.1524" layer="91" />
                <label x="83.82" y="43.18" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="GND" />
                <wire x1="132.08" y1="88.9" x2="129.54" y2="88.9" width="0.1524" layer="91" />
                <label x="129.54" y="88.9" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="GND" />
                <wire x1="132.08" y1="43.18" x2="129.54" y2="43.18" width="0.1524" layer="91" />
                <label x="129.54" y="43.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="RS" />
                <wire x1="132.08" y1="58.42" x2="129.54" y2="58.42" width="0.1524" layer="91" />
                <label x="129.54" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="GND" />
                <wire x1="228.6" y1="114.3" x2="231.14" y2="114.3" width="0.1524" layer="91" />
                <label x="231.14" y="114.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="GND" />
                <wire x1="228.6" y1="73.66" x2="231.14" y2="73.66" width="0.1524" layer="91" />
                <label x="231.14" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C9" gate="G$1" pin="2" />
                <wire x1="223.52" y1="27.94" x2="226.06" y2="27.94" width="0.1524" layer="91" />
                <label x="226.06" y="27.94" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C6" gate="G$1" pin="2" />
                <wire x1="35.56" y1="20.32" x2="38.1" y2="20.32" width="0.1524" layer="91" />
                <label x="38.1" y="20.32" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C7" gate="G$1" pin="2" />
                <wire x1="91.44" y1="20.32" x2="93.98" y2="20.32" width="0.1524" layer="91" />
                <label x="93.98" y="20.32" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C8" gate="G$1" pin="2" />
                <wire x1="147.32" y1="20.32" x2="149.86" y2="20.32" width="0.1524" layer="91" />
                <label x="149.86" y="20.32" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="2" />
                <wire x1="203.2" y1="15.24" x2="205.74" y2="15.24" width="0.1524" layer="91" />
                <label x="205.74" y="15.24" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="2" />
                <wire x1="35.56" y1="7.62" x2="38.1" y2="7.62" width="0.1524" layer="91" />
                <label x="38.1" y="7.62" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="2" />
                <wire x1="203.2" y1="2.54" x2="205.74" y2="2.54" width="0.1524" layer="91" />
                <label x="205.74" y="2.54" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP1" gate="G$1" pin="TP" />
                <wire x1="332.74" y1="187.96" x2="330.2" y2="187.96" width="0.1524" layer="91" />
                <label x="330.2" y="187.96" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO18" />
                <wire x1="81.28" y1="68.58" x2="83.82" y2="68.58" width="0.1524" layer="91" />
                <label x="83.82" y="68.58" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="TXD" />
                <wire x1="132.08" y1="114.3" x2="129.54" y2="114.3" width="0.1524" layer="91" />
                <label x="129.54" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO19" />
                <wire x1="81.28" y1="73.66" x2="83.82" y2="73.66" width="0.1524" layer="91" />
                <label x="83.82" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="RXD" />
                <wire x1="132.08" y1="109.22" x2="129.54" y2="109.22" width="0.1524" layer="91" />
                <label x="129.54" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_SILENT" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO23" />
                <wire x1="81.28" y1="93.98" x2="83.82" y2="93.98" width="0.1524" layer="91" />
                <label x="83.82" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="S" />
                <wire x1="132.08" y1="104.14" x2="129.54" y2="104.14" width="0.1524" layer="91" />
                <label x="129.54" y="104.14" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="1" />
                <wire x1="25.4" y1="7.62" x2="22.86" y2="7.62" width="0.1524" layer="91" />
                <label x="22.86" y="7.62" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="GAUGE_TX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO20" />
                <wire x1="81.28" y1="78.74" x2="83.82" y2="78.74" width="0.1524" layer="91" />
                <label x="83.82" y="78.74" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="D" />
                <wire x1="132.08" y1="68.58" x2="129.54" y2="68.58" width="0.1524" layer="91" />
                <label x="129.54" y="68.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R2" gate="G$1" pin="2" />
                <wire x1="91.44" y1="7.62" x2="93.98" y2="7.62" width="0.1524" layer="91" />
                <label x="93.98" y="7.62" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="GAUGE_RX" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO21" />
                <wire x1="81.28" y1="83.82" x2="83.82" y2="83.82" width="0.1524" layer="91" />
                <label x="83.82" y="83.82" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="R" />
                <wire x1="132.08" y1="63.5" x2="129.54" y2="63.5" width="0.1524" layer="91" />
                <label x="129.54" y="63.5" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="VBAT_SENSE" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="IO2" />
                <wire x1="40.64" y1="58.42" x2="38.1" y2="58.42" width="0.1524" layer="91" />
                <label x="38.1" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R8" gate="G$1" pin="2" />
                <wire x1="147.32" y1="7.62" x2="149.86" y2="7.62" width="0.1524" layer="91" />
                <label x="149.86" y="7.62" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R9" gate="G$1" pin="1" />
                <wire x1="193.04" y1="2.54" x2="190.5" y2="2.54" width="0.1524" layer="91" />
                <label x="190.5" y="2.54" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C10" gate="G$1" pin="1" />
                <wire x1="193.04" y1="15.24" x2="190.5" y2="15.24" width="0.1524" layer="91" />
                <label x="190.5" y="15.24" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP4" gate="G$1" pin="TP" />
                <wire x1="332.74" y1="157.48" x2="330.2" y2="157.48" width="0.1524" layer="91" />
                <label x="330.2" y="157.48" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="RST" class="0">
              <segment>
                <pinref part="U4" gate="G$1" pin="RST" />
                <wire x1="40.64" y1="109.22" x2="38.1" y2="109.22" width="0.1524" layer="91" />
                <label x="38.1" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="TP5" gate="G$1" pin="TP" />
                <wire x1="332.74" y1="147.32" x2="330.2" y2="147.32" width="0.1524" layer="91" />
                <label x="330.2" y="147.32" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_CANH" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="3" />
                <wire x1="25.4" y1="185.42" x2="22.86" y2="185.42" width="0.1524" layer="91" />
                <label x="22.86" y="185.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="CANH" />
                <wire x1="162.56" y1="114.3" x2="165.1" y2="114.3" width="0.1524" layer="91" />
                <label x="165.1" y="114.3" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="IO1" />
                <wire x1="208.28" y1="114.3" x2="205.74" y2="114.3" width="0.1524" layer="91" />
                <label x="205.74" y="114.3" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R3" gate="G$1" pin="1" />
                <wire x1="213.36" y1="93.98" x2="210.82" y2="93.98" width="0.1524" layer="91" />
                <label x="210.82" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="ECU_CANL" class="0">
              <segment>
                <pinref part="J10" gate="G$1" pin="4" />
                <wire x1="25.4" y1="180.34" x2="22.86" y2="180.34" width="0.1524" layer="91" />
                <label x="22.86" y="180.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U2" gate="G$1" pin="CANL" />
                <wire x1="162.56" y1="109.22" x2="165.1" y2="109.22" width="0.1524" layer="91" />
                <label x="165.1" y="109.22" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D4" gate="G$1" pin="IO2" />
                <wire x1="208.28" y1="109.22" x2="205.74" y2="109.22" width="0.1524" layer="91" />
                <label x="205.74" y="109.22" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="JP1" gate="G$1" pin="2" />
                <wire x1="274.32" y1="93.98" x2="276.86" y2="93.98" width="0.1524" layer="91" />
                <label x="276.86" y="93.98" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="ECU_TERM" class="0">
              <segment>
                <pinref part="R3" gate="G$1" pin="2" />
                <wire x1="223.52" y1="93.98" x2="226.06" y2="93.98" width="0.1524" layer="91" />
                <label x="226.06" y="93.98" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP1" gate="G$1" pin="1" />
                <wire x1="264.16" y1="93.98" x2="261.62" y2="93.98" width="0.1524" layer="91" />
                <label x="261.62" y="93.98" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANH" class="0">
              <segment>
                <pinref part="J11" gate="G$1" pin="3" />
                <wire x1="269.24" y1="149.86" x2="266.7" y2="149.86" width="0.1524" layer="91" />
                <label x="266.7" y="149.86" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="CANH" />
                <wire x1="162.56" y1="68.58" x2="165.1" y2="68.58" width="0.1524" layer="91" />
                <label x="165.1" y="68.58" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="IO1" />
                <wire x1="208.28" y1="73.66" x2="205.74" y2="73.66" width="0.1524" layer="91" />
                <label x="205.74" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R4" gate="G$1" pin="1" />
                <wire x1="213.36" y1="53.34" x2="210.82" y2="53.34" width="0.1524" layer="91" />
                <label x="210.82" y="53.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R5" gate="G$1" pin="1" />
                <wire x1="264.16" y1="53.34" x2="261.62" y2="53.34" width="0.1524" layer="91" />
                <label x="261.62" y="53.34" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANL" class="0">
              <segment>
                <pinref part="J11" gate="G$1" pin="4" />
                <wire x1="269.24" y1="144.78" x2="266.7" y2="144.78" width="0.1524" layer="91" />
                <label x="266.7" y="144.78" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U3" gate="G$1" pin="CANL" />
                <wire x1="162.56" y1="63.5" x2="165.1" y2="63.5" width="0.1524" layer="91" />
                <label x="165.1" y="63.5" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D5" gate="G$1" pin="IO2" />
                <wire x1="208.28" y1="68.58" x2="205.74" y2="68.58" width="0.1524" layer="91" />
                <label x="205.74" y="68.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R6" gate="G$1" pin="2" />
                <wire x1="223.52" y1="40.64" x2="226.06" y2="40.64" width="0.1524" layer="91" />
                <label x="226.06" y="40.64" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R7" gate="G$1" pin="2" />
                <wire x1="274.32" y1="40.64" x2="276.86" y2="40.64" width="0.1524" layer="91" />
                <label x="276.86" y="40.64" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="TERM_MID" class="0">
              <segment>
                <pinref part="R4" gate="G$1" pin="2" />
                <wire x1="223.52" y1="53.34" x2="226.06" y2="53.34" width="0.1524" layer="91" />
                <label x="226.06" y="53.34" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R5" gate="G$1" pin="2" />
                <wire x1="274.32" y1="53.34" x2="276.86" y2="53.34" width="0.1524" layer="91" />
                <label x="276.86" y="53.34" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="R6" gate="G$1" pin="1" />
                <wire x1="213.36" y1="40.64" x2="210.82" y2="40.64" width="0.1524" layer="91" />
                <label x="210.82" y="40.64" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R7" gate="G$1" pin="1" />
                <wire x1="264.16" y1="40.64" x2="261.62" y2="40.64" width="0.1524" layer="91" />
                <label x="261.62" y="40.64" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C9" gate="G$1" pin="1" />
                <wire x1="213.36" y1="27.94" x2="210.82" y2="27.94" width="0.1524" layer="91" />
                <label x="210.82" y="27.94" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
          </nets>
        </sheet>
      </sheets>
    </schematic>
  </drawing>
</eagle>
