`timescale 1ns / 1ps

module vi_sync_gen_xy(
					//           input           //
					input 			sys_rst_n,         // reset
					input 			vp_clk_in, 			 // clock
					input [15:0]	vp_data_in,			 // input video data
														
					//				 output          //
					output reg		dvalid_flag,       
					output reg		vsync_flag,
					output reg		hsync_flag,
					output 			vsync_ad,
					output 			vsync_ad_ver,
					output			hsync_ad,
					output [15:0]	video_data,
					output reg [10:0] x,
					output reg [10:0] y,
					output 		  	this2out
			);
	 //----------------------------------------------------------//
	 //-------------------Local declarations---------------------//
	 //----------------------------------------------------------//
	 reg               vsync_flag_buf;
	 reg               hsync_flag_buf;
	
	 wire 					hsync_ad_pos;
	 wire						vsync_ad_neg;
	 wire 	            xy_b7;
	 wire 	            xy_f;
	 wire 	            xy_v;
	 wire 	            xy_h;
	 wire 	            xy_p3;
	 wire 	            xy_p2;
	 wire 	            xy_p1;
	 wire 	            xy_p0;   
	 wire 	            is_xy;

	 reg    [15 : 0]   pdata_t1;
	 reg    [15 : 0]   pdata_t2;
	 reg    [15 : 0]   pdata_t3;
	 reg    [15 : 0]   pdata_t4;
	 
	 wire              eav_sav_pulse;
	 wire              valid_sav;
	 wire              valid_eav; 
	 wire              blank_sav; 
	 wire              blank_eav;	
	 wire              hsync_ad_blank;
	 reg    [5  : 0 ]  hsync_ad_blank_cnt;
	 reg               hsync_ad_blank_num;
	 reg               hsync_ad_blank_num_buf1;
	 reg               hsync_ad_blank_num_buf2;
	 reg               hsync_ad_blank_num_buf3;
	 reg               hsync_ad_blank_num_buf4;
	 reg               hsync_ad_blank_num_buf5;
	 reg               hsync_ad_blank_num_buf6;

   //------------------------------------- vp_clk_in domain---------------------------------------//   
   //------ Extract the synchronization from video stream in BT1120 format------//	
   
   assign 	xy_b7 = pdata_t1[7];
   assign 	xy_f  = pdata_t1[6];
   assign 	xy_v  = pdata_t1[5];
   assign 	xy_h  = pdata_t1[4];
   assign 	xy_p3 = pdata_t1[3];
   assign 	xy_p2 = pdata_t1[2];
   assign 	xy_p1 = pdata_t1[1];
   assign 	xy_p0 = pdata_t1[0];
   
   assign 	is_xy = (xy_b7==1'b1) & (xy_p3==xy_v^xy_h) & (xy_p2==xy_f^xy_h) & (xy_p1==xy_f^xy_v) & (xy_p0==xy_f^xy_v^xy_h);
      
   always @(posedge vp_clk_in or negedge sys_rst_n)
     begin
	    if(!sys_rst_n)
	      begin
	        pdata_t1	<=   8'h00;
	        pdata_t2	<=   8'h00;
	        pdata_t3	<=   8'h00;
	        pdata_t4	<=   8'h00;
	      end
	    else 
	      begin
	        pdata_t1	<=   vp_data_in;
	        pdata_t2	<=   pdata_t1;
	        pdata_t3	<=   pdata_t2;
	        pdata_t4	<=   pdata_t3;
	      end
     end 
     
   //produce sorts of EAV and SAV according to the protocol of SMPTE294
   assign eav_sav_pulse = (&pdata_t4[7:0]) & (!(|pdata_t3[7:0])) & (!(pdata_t2[7:0])) & is_xy;
   assign valid_sav = eav_sav_pulse & (~xy_f) & (~xy_v) & (~xy_h);//8080
   assign valid_eav = eav_sav_pulse & (~xy_f) & (~xy_v) & (xy_h); //9D9D
   assign blank_sav = eav_sav_pulse & (~xy_f) & (xy_v) & (~xy_h); //ABAB
   assign blank_eav = eav_sav_pulse & (~xy_f) & (xy_v) & (xy_h);  //B6B6

   // produce dvalid_flag       每一行中，在读有效数据时，就OK, dvalid_flag 为1，否则为0
   always  @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       dvalid_flag  <=  1'b0;
     else  if ( valid_sav )
       dvalid_flag  <=  1'b1;
     else  if ( valid_eav | blank_eav )
       dvalid_flag  <=  1'b0;
     else 
       dvalid_flag  <= dvalid_flag;

  //-----------------------vsync & hsync-----------------------------//
	// produce vsync_flag			有效行时，为1。 到非有效行时为0， 对于y是有效控制 y++
   always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       vsync_flag  <=  1'b0;
     else  if ( valid_sav | valid_eav )
       vsync_flag  <=  1'b1;
     else  if ( blank_eav )
       vsync_flag  <=  1'b0;
     else 
       vsync_flag  <= vsync_flag;

   // produce vsync_ad, it is one vp_clk_in pulse width of vsync_flag's posedge
   always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       begin
         vsync_flag_buf    <=  1'b0;
       end                 
     else
       begin
         vsync_flag_buf    <= vsync_flag;
       end

                
   assign  vsync_ad     =  vsync_flag && (!vsync_flag_buf);   //positive edge of vsynn_flag
	assign  vsync_ad_neg =  (!vsync_flag) && vsync_flag_buf;	//negedge edge of vsync_flah 有效数据结束信号
   //assign  vsync_ad_ver =  vsync_flag_buf5 && (!vsync_flag);//negative edge of vsync_flag,lenthen the width to 6'clk for PCI clk domain
   //assign  vsync_ad_ver =  vsync_flag && (!vsync_flag_buf5);  //test
   //我要写下处理y的逻辑
	reg [10:0] max_y;	//需要知道场扫描中，宽度
	always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       begin
         y    <=  1'b0;
			max_y <= 1'b0;
       end                 
     else
       begin
			if (vsync_ad)
				y <= 1'b0;
			else begin
				if (vsync_flag) //在读取有效行时
					//每读完一行 y自增，其它时间维持原值
						if ( hsync_ad)
							y <= y + 1'b1;
						else
							y <= y;
				else if(vsync_ad_neg) begin
					max_y <= y;
					y <= 1'b0;
				end
				else
					y <= 1'b0;
			end
       end
	//再写一个处理x的逻辑
	reg state_x;
	always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       begin
         x    <=  1'b0;
			state_x <= 1'b0;
       end                 
     else
       begin
			if (vsync_ad)
				x <= 1'b0;
			else begin
				if (vsync_flag) //在读取有效行时
					//每读完一个有效字 x自增，其它时间维持原值
						if ( hsync_ad) begin
							state_x <= 1'b0;
							x <= 1'b0;
						end
						else if (hsync_ad_pos) begin
							state_x <= 1'b1;
							x <= 1'b0;
						end
						else begin
							if ( state_x)
								x <= x + 1'b1;//1'b0;
							else
								x <= 1'b0;
						end
				else
					x <= 1'b0;
			end
       end
	//还要加一个状态机FSM，用于确定this2out的值，只有合适的节拍，对video_data进行采集，才能用signaltab以日志的方式记录
	reg [10:0] last_y;
	reg [15:0] ticks;
	parameter  V1080P30_TICK  = 16'd8;////16'd30;

	//隔一定的时间，取图像中一行
	always @ ( posedge vp_clk_in  or negedge sys_rst_n)
	  if ( !sys_rst_n )
		 begin
			last_y    <=  1'b0;
			ticks		 <= 1'b0;
		 end                 
	  else begin			
			if ( ticks >= V1080P30_TICK)
				begin
					ticks <= 1'b0;
					if ( last_y >= max_y )
						last_y <= 1'b0;
					else
						last_y <= last_y + 3'd1;
				end
			else
				if ( vsync_ad)
					ticks <= ticks + 1'b1;
				else
					ticks <= ticks;
	  end
	
	//定义thisout的逻辑
	assign this2out = (last_y == y && vsync_flag && x > 11'd160);
   
   // produce hsync_flag
   always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
       hsync_flag  <=  1'b0;
     else  if ( valid_sav )
       hsync_flag  <=  1'b1;
     else  if ( valid_eav | blank_eav )
       hsync_flag  <=  1'b0;
     else
       hsync_flag  <= hsync_flag;

   // produce hsync_ad, it is one vp_clk_in pulse width of hsync_flag's negedge
   always @ ( posedge vp_clk_in  or negedge sys_rst_n)
     if ( !sys_rst_n )
	      hsync_flag_buf  <=  1'b0;
     else
	      hsync_flag_buf  <=  hsync_flag;
	
   assign  hsync_ad  =  hsync_flag_buf && (!hsync_flag);
	assign  hsync_ad_pos = hsync_flag && (!hsync_flag_buf);
   assign  video_data = pdata_t1;   		
endmodule
