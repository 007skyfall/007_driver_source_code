1、iic相关概念
   速度：低10k/s   标100k    中400k    高3.4m
   传输方式：半双工
   相关硬件：	
		硬件：i2c主机控制器（struct i2c_adapter）     i2c的从设备(struct i2c_client)
		软件：i2c控制器驱动(platform)     		      i2c的设备驱动(struct i2c_driver)
	
   主从识别：根据从设备地址
   
2、协议：i2c起始，停止，应答，非应答；
3、目录:/linux3.14/driver/i2c/
			algos:封装了i2c的传输方法
			busses:i2c控制器的驱动
			muxes:i2c奇葩功能 
			
		框架分析：	
			app:	open 	read	 write	 close
					-------------------------------------
			kernel:i2c设备驱动i2c_drvier：
					drv_open  drv_read drv_write drv_close
					--------------------------------------
					           i2c核心层
					--------------------------------------
					         i2c控制器驱动
					--------------------------------------
		   hardware:           i2c控制器
		            --------------------------------------
					          i2c_slaves....
			
		 结构体分析：
			i2c控制器：struct i2c_adapter		
				struct i2c_adapter {
						 struct module *owner;
						 const 	struct i2c_algorithm *algo; /*该总线上的通信方法 ：时钟控制，start/stop,中断等*/
						 struct device dev;   		/*适配器设备*/
						 int 	nr;                 /*总线的编号*/
						 char 	name[48];       	/*名字 */
				 }；
				  struct i2c_algorithm {
					/*消息发送函数指针 ，不同适配器其实现不一样*/
					 int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,int num); 		 
				};
				int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
				功能：i2c数据传输
				参数：adap:struct i2c_adapter指针对象
				       msgs:待传输的i2c消息
				       num：待传输的消息个数
				返回值：Returns negative errno, else the number of messages executed
				struct i2c_msg {
						__u16 addr;	/*从设备地址*/
						__u16 flags;/*一般表征读写*/
							#define I2C_M_RD		0x0001	/* read data, from slave to master */
						__u16 len;		/* 数据长度*/
						__u8 *buf;		/* 待发送的数据地址*/
				};

			i2c设备：
				struct i2c_client {   
						 unsigned short addr;   		/*I2C设备在总线上的的地址 */ 
						 char name[I2C_NAME_SIZE]; 		/*设备名*/
						 struct i2c_adapter *adapter;   /*指向该I2C设备挂载的I2C适配器*/
						 struct i2c_driver *driver;     /*指向支持该I2C设备的驱动*/
						 struct device dev;             /*用于总线设备驱动模型*/
						 int irq;                      	/*该设备能产生的中断号*/
				};
				
				(1)devicetree：
					i2c控制器
					i2c slave
						i2c_adapter{
								xxx= xxx;
								lll = lll;
								
								i2c_slave{
									
								};
						};
					
				(2)struct i2c_board_info {
						char		type[I2C_NAME_SIZE]；//名称
						unsigned short	addr;//i2c slave addr
						int		irq;
					};
					
				int __init i2c_register_board_info(int busnum,struct i2c_board_info const *info, unsigned len)
				功能：注册i2c_board_info
				参数：busnum:该设备所在的总线标号
				      info:struct i2c_board_info的指针对象
				      len：待注册的个数
				返回值：（一般不需要判断返回值）成功：0  失败：-ERRNO
	
			i2c驱动：
				struct i2c_driver {
					int (*probe)(struct i2c_client *, const struct i2c_device_id *);//当驱动与设备信息匹配时，自动执行
					int (*remove)(struct i2c_client *);//当驱动或设备有任意一方移除时，自动执行
					struct device_driver driver;//驱动对象
					const struct i2c_device_id *id_table;//i2c驱动所支持的设备列表
				};
				
				struct device_driver {
					const char		*name;//名称
					struct module		*owner;
					const struct of_device_id	*of_match_table;//设备树匹配
				};
				struct of_device_id
				{
					char	compatible[128];
				};
				
				struct i2c_device_id {
					char name[I2C_NAME_SIZE];
				};
				
		函数API:
			（1）i2c_add_driver（struct i2c_driver *driver）
				功能：注册i2c_driver
				参数：driver:struct i2c_driver的指针对象
				返回值：成功：0  失败：-ERRNO
				
			（2）void i2c_del_driver(struct i2c_driver *driver)
				功能：注销i2c_driver
				参数：driver:struct i2c_driver的指针对象
				返回值：无
				
				
		i2c设备驱动:
		/*
			想要将i2c的设备信息以设备树的形式去提供，参考内核中关于i2c设备的设备树去写：
				i2c_7: i2c@138D0000{
					#address-cells=<1>;
					#size-cells=<0>;

					samsung,i2c-sda-delay=<100>;
					samsung,i2c-max-bus-freq=<20000>;
					pinctrl-0=<&i2c7_bus>;
					pinctrl-names="default";
					status="okay";
		 
						ov3640: sensor@3c{
							compatible = "samsung,OV3640";//在i2c_driver中的与设备树的匹配项
							reg = <0x3c>;//i2c的从机地址
						}; 
				};			
		*/
			(1)提供设备信息：原理图 + 数据手册
				mpu6050  + i2c5  + addr0x68
				
			(2)提供设备信息
				i2c_5: i2c@138B0000{
					#address-cells=<1>;
					#size-cells=<0>;

					samsung,i2c-sda-delay=<100>;
					samsung,i2c-max-bus-freq=<20000>;
					pinctrl-0=<&i2c5_bus>;
					pinctrl-names="default";
					status="okay";
		 
						mpu6050{
							compatible = "farsight,mpu6050";//在i2c_driver中的与设备树的匹配项
							reg = <0x68>;//i2c的从机地址
						}; 
				};	
				
				 在linux内核顶层目录执行：make dtbs
				 将 xxx.dtb  拷贝至 /tftpboot
			

4.程序编写流程
	（1）模块三要素
	（2）struct i2c_driver的定义 + 填充
	（3）init ->注册
				probe：
					对mpu6050进行相关配置工作
					
				因为传感器要提供对应的数据，app要读，底层提供读
				    cdev相关
	（4）exit ->注销

 
 
 

 
 
 
 
 
 
 
 
 
 

		  