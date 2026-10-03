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
            <package name="SMA">
              <description>Generic land pattern. Replace with the LCSC part's own footprint before layout. Pad 1 is the cathode.</description>
              <smd name="1" x="-2" y="0" dx="2.2" dy="1.7" layer="1" />
              <smd name="2" x="2" y="0" dx="2.2" dy="1.7" layer="1" />
              <text x="-2" y="1.25" size="1.016" layer="25">&gt;NAME</text>
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
            <package name="HDR-1X8">
              <description>Pin header, 2.54 mm pitch, 1.0 mm drill.</description>
              <pad name="1" x="-8.89" y="0" drill="1" diameter="1.8" shape="square" />
              <pad name="2" x="-6.35" y="0" drill="1" diameter="1.8" />
              <pad name="3" x="-3.81" y="0" drill="1" diameter="1.8" />
              <pad name="4" x="-1.27" y="0" drill="1" diameter="1.8" />
              <pad name="5" x="1.27" y="0" drill="1" diameter="1.8" />
              <pad name="6" x="3.81" y="0" drill="1" diameter="1.8" />
              <pad name="7" x="6.35" y="0" drill="1" diameter="1.8" />
              <pad name="8" x="8.89" y="0" drill="1" diameter="1.8" />
              <wire x1="-10.16" y1="1.27" x2="10.16" y2="1.27" width="0.127" layer="21" />
              <wire x1="10.16" y1="1.27" x2="10.16" y2="-1.27" width="0.127" layer="21" />
              <wire x1="10.16" y1="-1.27" x2="-10.16" y2="-1.27" width="0.127" layer="21" />
              <wire x1="-10.16" y1="-1.27" x2="-10.16" y2="1.27" width="0.127" layer="21" />
              <text x="-10.16" y="1.7" size="1.016" layer="25">&gt;NAME</text>
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
          </packages>
          <symbols>
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
            <symbol name="GAUGE-HEADER">
              <wire x1="-7.62" y1="21.59" x2="7.62" y2="21.59" width="0.254" layer="94" />
              <wire x1="7.62" y1="21.59" x2="7.62" y2="-21.59" width="0.254" layer="94" />
              <wire x1="7.62" y1="-21.59" x2="-7.62" y2="-21.59" width="0.254" layer="94" />
              <wire x1="-7.62" y1="-21.59" x2="-7.62" y2="21.59" width="0.254" layer="94" />
              <pin name="VBUS" x="12.7" y="17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="GND" x="12.7" y="12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="3V3" x="12.7" y="7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="UART_A" x="12.7" y="2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="UART_B" x="12.7" y="-2.54" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="POS6" x="12.7" y="-7.62" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="POS7" x="12.7" y="-12.7" visible="pin" length="middle" direction="pas" rot="R180" />
              <pin name="POS8" x="12.7" y="-17.78" visible="pin" length="middle" direction="pas" rot="R180" />
              <text x="-7.62" y="22.59" size="1.778" layer="95">&gt;NAME</text>
              <text x="-7.62" y="-24.39" size="1.778" layer="96">&gt;VALUE</text>
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
            <deviceset name="GAUGE-HEADER" prefix="P" uservalue="yes">
              <gates>
                <gate name="G$1" symbol="GAUGE-HEADER" x="0" y="0" />
              </gates>
              <devices>
                <device name="" package="HDR-1X8">
                  <connects>
                    <connect gate="G$1" pin="VBUS" pad="1" />
                    <connect gate="G$1" pin="GND" pad="2" />
                    <connect gate="G$1" pin="3V3" pad="3" />
                    <connect gate="G$1" pin="UART_A" pad="4" />
                    <connect gate="G$1" pin="UART_B" pad="5" />
                    <connect gate="G$1" pin="POS6" pad="6" />
                    <connect gate="G$1" pin="POS7" pad="7" />
                    <connect gate="G$1" pin="POS8" pad="8" />
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
          </devicesets>
        </library>
      </libraries>
      <attributes />
      <variantdefs />
      <classes>
        <class number="0" name="default" width="0" drill="0" />
      </classes>
      <parts>
        <part name="JP1" library="haltech-gauges" deviceset="JUMPER-2" device="" value="CHAIN PWR" />
        <part name="D1" library="haltech-gauges" deviceset="SCHOTTKY-SMA" device="" value="SS14">
          <attribute name="LCSC" value="C2480" />
        </part>
        <part name="R1" library="haltech-gauges" deviceset="R0805" device="" value="10k">
          <attribute name="LCSC" value="C17414" />
        </part>
        <part name="C1" library="haltech-gauges" deviceset="C0805" device="" value="100n">
          <attribute name="LCSC" value="C49678" />
        </part>
        <part name="P1" library="haltech-gauges" deviceset="GAUGE-HEADER" device="" value="1x8 male 2.54" />
        <part name="U1" library="haltech-gauges" deviceset="SN65HVD230" device="" value="SN65HVD230DR">
          <attribute name="LCSC" value="C12084" />
        </part>
        <part name="D2" library="haltech-gauges" deviceset="PESD2CAN" device="" value="PESD2CAN">
          <attribute name="LCSC" value="C75176" />
        </part>
        <part name="TP1" library="haltech-gauges" deviceset="TESTPOINT" device="" value="SPARE" />
        <part name="J1" library="haltech-gauges" deviceset="MICROFIT-2X2" device="" value="43045-0412 IN" />
        <part name="J2" library="haltech-gauges" deviceset="MICROFIT-2X2" device="" value="43045-0412 OUT" />
      </parts>
      <sheets>
        <sheet>
          <plain>
            <text x="10.16" y="160.24" size="2.54" layer="97">Haltech gauges: gauge adapter (one per gauge)</text>
            <text x="10.16" y="154.24" size="1.778" layer="97">J1 and J2 are wired pin for pin; the chain passes straight through.</text>
            <text x="10.16" y="151.24" size="1.778" layer="97">P1 plugs into the gauge's rear 1x8 socket. POS6/POS7/POS8 carry GPIO16/17/18 in an</text>
            <text x="10.16" y="148.24" size="1.778" layer="97">order confirmed at bring-up; the firmware maps CAN TX/RX to whichever land on 6 and 7.</text>
            <text x="10.16" y="145.24" size="1.778" layer="97">No termination here: the last adapter's J2 takes a 120R terminator plug.</text>
            <text x="10.16" y="142.24" size="1.778" layer="97">Footprints are generic. Assign each part its LCSC number and use that footprint.</text>
          </plain>
          <instances>
            <instance part="JP1" gate="G$1" x="45.72" y="111.76" />
            <instance part="D1" gate="G$1" x="91.44" y="111.76" />
            <instance part="R1" gate="G$1" x="137.16" y="111.76" />
            <instance part="C1" gate="G$1" x="182.88" y="111.76" />
            <instance part="P1" gate="G$1" x="35.56" y="66.04" />
            <instance part="U1" gate="G$1" x="111.76" y="66.04" />
            <instance part="D2" gate="G$1" x="111.76" y="20.32" />
            <instance part="TP1" gate="G$1" x="50.8" y="20.32" />
            <instance part="J1" gate="G$1" x="193.04" y="81.28" />
            <instance part="J2" gate="G$1" x="193.04" y="40.64" />
          </instances>
          <busses />
          <nets>
            <net name="+5V_CHAIN" class="0">
              <segment>
                <pinref part="J1" gate="G$1" pin="1" />
                <wire x1="182.88" y1="88.9" x2="180.34" y2="88.9" width="0.1524" layer="91" />
                <label x="180.34" y="88.9" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J2" gate="G$1" pin="1" />
                <wire x1="182.88" y1="48.26" x2="180.34" y2="48.26" width="0.1524" layer="91" />
                <label x="180.34" y="48.26" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="D1" gate="G$1" pin="A" />
                <wire x1="86.36" y1="111.76" x2="83.82" y2="111.76" width="0.1524" layer="91" />
                <label x="83.82" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="VBUS_D" class="0">
              <segment>
                <pinref part="D1" gate="G$1" pin="K" />
                <wire x1="96.52" y1="111.76" x2="99.06" y2="111.76" width="0.1524" layer="91" />
                <label x="99.06" y="111.76" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="JP1" gate="G$1" pin="1" />
                <wire x1="40.64" y1="111.76" x2="38.1" y2="111.76" width="0.1524" layer="91" />
                <label x="38.1" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="VBUS" class="0">
              <segment>
                <pinref part="JP1" gate="G$1" pin="2" />
                <wire x1="50.8" y1="111.76" x2="53.34" y2="111.76" width="0.1524" layer="91" />
                <label x="53.34" y="111.76" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="P1" gate="G$1" pin="VBUS" />
                <wire x1="48.26" y1="83.82" x2="50.8" y2="83.82" width="0.1524" layer="91" />
                <label x="50.8" y="83.82" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="GND" class="0">
              <segment>
                <pinref part="J1" gate="G$1" pin="2" />
                <wire x1="182.88" y1="83.82" x2="180.34" y2="83.82" width="0.1524" layer="91" />
                <label x="180.34" y="83.82" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J2" gate="G$1" pin="2" />
                <wire x1="182.88" y1="43.18" x2="180.34" y2="43.18" width="0.1524" layer="91" />
                <label x="180.34" y="43.18" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="P1" gate="G$1" pin="GND" />
                <wire x1="48.26" y1="78.74" x2="50.8" y2="78.74" width="0.1524" layer="91" />
                <label x="50.8" y="78.74" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="GND" />
                <wire x1="96.52" y1="53.34" x2="93.98" y2="53.34" width="0.1524" layer="91" />
                <label x="93.98" y="53.34" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="RS" />
                <wire x1="96.52" y1="68.58" x2="93.98" y2="68.58" width="0.1524" layer="91" />
                <label x="93.98" y="68.58" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="2" />
                <wire x1="187.96" y1="111.76" x2="190.5" y2="111.76" width="0.1524" layer="91" />
                <label x="190.5" y="111.76" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D2" gate="G$1" pin="GND" />
                <wire x1="121.92" y1="22.86" x2="124.46" y2="22.86" width="0.1524" layer="91" />
                <label x="124.46" y="22.86" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="+3V3" class="0">
              <segment>
                <pinref part="P1" gate="G$1" pin="3V3" />
                <wire x1="48.26" y1="73.66" x2="50.8" y2="73.66" width="0.1524" layer="91" />
                <label x="50.8" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="VCC" />
                <wire x1="96.52" y1="58.42" x2="93.98" y2="58.42" width="0.1524" layer="91" />
                <label x="93.98" y="58.42" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="C1" gate="G$1" pin="1" />
                <wire x1="177.8" y1="111.76" x2="175.26" y2="111.76" width="0.1524" layer="91" />
                <label x="175.26" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="1" />
                <wire x1="132.08" y1="111.76" x2="129.54" y2="111.76" width="0.1524" layer="91" />
                <label x="129.54" y="111.76" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CAN_TX" class="0">
              <segment>
                <pinref part="P1" gate="G$1" pin="POS6" />
                <wire x1="48.26" y1="58.42" x2="50.8" y2="58.42" width="0.1524" layer="91" />
                <label x="50.8" y="58.42" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="D" />
                <wire x1="96.52" y1="78.74" x2="93.98" y2="78.74" width="0.1524" layer="91" />
                <label x="93.98" y="78.74" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="R1" gate="G$1" pin="2" />
                <wire x1="142.24" y1="111.76" x2="144.78" y2="111.76" width="0.1524" layer="91" />
                <label x="144.78" y="111.76" size="1.778" layer="95" />
              </segment>
            </net>
            <net name="CAN_RX" class="0">
              <segment>
                <pinref part="P1" gate="G$1" pin="POS7" />
                <wire x1="48.26" y1="53.34" x2="50.8" y2="53.34" width="0.1524" layer="91" />
                <label x="50.8" y="53.34" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="R" />
                <wire x1="96.52" y1="73.66" x2="93.98" y2="73.66" width="0.1524" layer="91" />
                <label x="93.98" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="SPARE" class="0">
              <segment>
                <pinref part="P1" gate="G$1" pin="POS8" />
                <wire x1="48.26" y1="48.26" x2="50.8" y2="48.26" width="0.1524" layer="91" />
                <label x="50.8" y="48.26" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="TP1" gate="G$1" pin="TP" />
                <wire x1="48.26" y1="20.32" x2="45.72" y2="20.32" width="0.1524" layer="91" />
                <label x="45.72" y="20.32" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANH" class="0">
              <segment>
                <pinref part="J1" gate="G$1" pin="3" />
                <wire x1="182.88" y1="78.74" x2="180.34" y2="78.74" width="0.1524" layer="91" />
                <label x="180.34" y="78.74" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J2" gate="G$1" pin="3" />
                <wire x1="182.88" y1="38.1" x2="180.34" y2="38.1" width="0.1524" layer="91" />
                <label x="180.34" y="38.1" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="CANH" />
                <wire x1="127" y1="78.74" x2="129.54" y2="78.74" width="0.1524" layer="91" />
                <label x="129.54" y="78.74" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D2" gate="G$1" pin="IO1" />
                <wire x1="101.6" y1="22.86" x2="99.06" y2="22.86" width="0.1524" layer="91" />
                <label x="99.06" y="22.86" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
            <net name="CANL" class="0">
              <segment>
                <pinref part="J1" gate="G$1" pin="4" />
                <wire x1="182.88" y1="73.66" x2="180.34" y2="73.66" width="0.1524" layer="91" />
                <label x="180.34" y="73.66" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="J2" gate="G$1" pin="4" />
                <wire x1="182.88" y1="33.02" x2="180.34" y2="33.02" width="0.1524" layer="91" />
                <label x="180.34" y="33.02" size="1.778" layer="95" rot="R180" />
              </segment>
              <segment>
                <pinref part="U1" gate="G$1" pin="CANL" />
                <wire x1="127" y1="73.66" x2="129.54" y2="73.66" width="0.1524" layer="91" />
                <label x="129.54" y="73.66" size="1.778" layer="95" />
              </segment>
              <segment>
                <pinref part="D2" gate="G$1" pin="IO2" />
                <wire x1="101.6" y1="17.78" x2="99.06" y2="17.78" width="0.1524" layer="91" />
                <label x="99.06" y="17.78" size="1.778" layer="95" rot="R180" />
              </segment>
            </net>
          </nets>
        </sheet>
      </sheets>
    </schematic>
  </drawing>
</eagle>
