module select_video_to_signaltab(
	input [5:0] sel,			//选择哪一个进行输出
	//-----------------视频源1----------------
	input rst1,
	input clk1,
	input [15:0]vdata1,
	//-----------------视频源2----------------
	input rst2,
	input clk2,
	input [15:0]vdata2,
	//-----------------视频源3----------------
	input rst3,
	input clk3,
	input [15:0]vdata3,
	//-----------------视频源4----------------
	input rst4,
	input clk4,
	input [15:0]vdata4,
	//-----------------视频源5----------------
	input rst5,
	input clk5,
	input [15:0]vdata5,
	//-----------------视频源6----------------
	input rst6,
	input clk6,
	input [15:0]vdata6,
	//-----------------视频源7----------------
	input rst7,
	input clk7,
	input [15:0]vdata7,
	//-----------------视频源8----------------
	input rst8,
	input clk8,
	input [15:0]vdata8,
	//-----------------视频源9----------------
	input rst9,
	input clk9,
	input [15:0]vdata9,
	//-----------------视频源a----------------
	input rstA,
	input clkA,
	input [15:0]vdataA,
	//-----------------视频源b----------------
	input rstB,
	input clkB,
	input [15:0]vdataB,
	//-----------------视频源c----------------
	input rstC,
	input clkC,
	input [15:0]vdataC,
	//-----------------视频源d----------------
	input rstD,
	input clkD,
	input [15:0]vdataD,
	//-----------------视频源e----------------
	input rstE,
	input clkE,
	input [15:0]vdataE,
	//-----------------视频源f----------------
	input rstF,
	input clkF,
	input [15:0]vdataF,
	//----------------输出--------------------
	output reg rst,
	output reg clk,
	output reg [15:0] videoData
);
	always@(*) begin
		case(sel)
			    6'd1 :  begin
					rst <= rst1;
					clk <= clk1;
					videoData <= vdata1;
					end
				 6'd2 : begin
					rst <= rst2;
					clk <= clk2;
					videoData <= vdata2;
					end
				 6'd3 : begin
					rst <= rst3;
					clk <= clk3;
					videoData <= vdata3;
					end
				 6'd4 : begin
					rst <= rst4;
					clk <= clk4;
					videoData <= vdata4;
					end
				 6'd5 : begin
					rst <= rst5;
					clk <= clk5;
					videoData <= vdata5;
					end
				 6'd6 : begin
					rst <= rst6;
					clk <= clk6;
					videoData <= vdata6;
					end
				 6'd7 : begin
					rst <= rst7;
					clk <= clk7;
					videoData <= vdata7;
					end
				 6'd8 : begin
					rst <= rst8;
					clk <= clk8;
					videoData <= vdata8;
					end
				  6'd9 : begin
					rst <= rst9;
					clk <= clk9;
					videoData <= vdata9;
					end
				  6'd10 : begin
					rst <= rstA;
					clk <= clkA;
					videoData <= vdataA;
					end
				  6'd11 : begin
					rst <= rstB;
					clk <= clkB;
					videoData <= vdataB;
					end
				  6'd12 : begin
					rst <= rstC;
					clk <= clkC;
					videoData <= vdataC;
					end
					 6'd13 : begin
					rst <= rstD;
					clk <= clkD;
					videoData <= vdataD;
					end
					 6'd14 : begin
					rst <= rstE;
					clk <= clkE;
					videoData <= vdataE;
					end
					 6'd15 : begin
					rst <= rstF;
					clk <= clkF;
					videoData <= vdataF;
					end
					default: begin
						rst <= rst1;
						clk <= clk1;
						videoData <= vdata1;
					end
          endcase	
	end
endmodule