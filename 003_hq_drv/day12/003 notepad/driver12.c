1��iic��ظ���
   �ٶȣ���10k/s   ��100k    ��400k    ��3.4m
   ���䷽ʽ����˫��
   ���Ӳ����	
		Ӳ����i2c������������struct i2c_adapter��     i2c�Ĵ��豸(struct i2c_client)
		�����i2c����������(platform)     		      i2c���豸����(struct i2c_driver)
	
   ����ʶ�𣺸��ݴ��豸��ַ
   
2��Э�飺i2c��ʼ��ֹͣ��Ӧ�𣬷�Ӧ��
3��Ŀ¼:/linux3.14/driver/i2c/
			algos:��װ��i2c�Ĵ��䷽��
			busses:i2c������������
			muxes:i2c���⹦�� 
			
		��ܷ�����	
			app:	open 	read	 write	 close
					-------------------------------------
			kernel:i2c�豸����i2c_drvier��
					drv_open  drv_read drv_write drv_close
					--------------------------------------
					           i2c���Ĳ�
					--------------------------------------
					         i2c����������
					--------------------------------------
		   hardware:           i2c������
		            --------------------------------------
					          i2c_slaves....
			
		 �ṹ�������
			i2c��������struct i2c_adapter		
				struct i2c_adapter {
						 struct module *owner;
						 const 	struct i2c_algorithm *algo; /*�������ϵ�ͨ�ŷ��� ��ʱ�ӿ��ƣ�start/stop,�жϵ�*/
						 struct device dev;   		/*�������豸*/
						 int 	nr;                 /*���ߵı��*/
						 char 	name[48];       	/*���� */
				 }��
				  struct i2c_algorithm {
					/*��Ϣ���ͺ���ָ�� ����ͬ��������ʵ�ֲ�һ��*/
					 int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,int num); 		 
				};
				int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
				���ܣ�i2c���ݴ���
				������adap:struct i2c_adapterָ�����
				       msgs:�������i2c��Ϣ
				       num�����������Ϣ����
				����ֵ��Returns negative errno, else the number of messages executed
				struct i2c_msg {
						__u16 addr;	/*���豸��ַ*/
						__u16 flags;/*һ�������д*/
							#define I2C_M_RD		0x0001	/* read data, from slave to master */
						__u16 len;		/* ���ݳ���*/
						__u8 *buf;		/* �����͵����ݵ�ַ*/
				};

			i2c�豸��
				struct i2c_client {   
						 unsigned short addr;   		/*I2C�豸�������ϵĵĵ�ַ */ 
						 char name[I2C_NAME_SIZE]; 		/*�豸��*/
						 struct i2c_adapter *adapter;   /*ָ���I2C�豸���ص�I2C������*/
						 struct i2c_driver *driver;     /*ָ��֧�ָ�I2C�豸������*/
						 struct device dev;             /*���������豸����ģ��*/
						 int irq;                      	/*���豸�ܲ������жϺ�*/
				};
				
				(1)devicetree��
					i2c������
					i2c slave
						i2c_adapter{
								xxx= xxx;
								lll = lll;
								
								i2c_slave{
									
								};
						};
					
				(2)struct i2c_board_info {
						char		type[I2C_NAME_SIZE]��//����
						unsigned short	addr;//i2c slave addr
						int		irq;
					};
					
				int __init i2c_register_board_info(int busnum,struct i2c_board_info const *info, unsigned len)
				���ܣ�ע��i2c_board_info
				������busnum:���豸���ڵ����߱��
				      info:struct i2c_board_info��ָ�����
				      len����ע��ĸ���
				����ֵ����һ�㲻��Ҫ�жϷ���ֵ���ɹ���0  ʧ�ܣ�-ERRNO
	
			i2c������
				struct i2c_driver {
					int (*probe)(struct i2c_client *, const struct i2c_device_id *);//���������豸��Ϣƥ��ʱ���Զ�ִ��
					int (*remove)(struct i2c_client *);//���������豸������һ���Ƴ�ʱ���Զ�ִ��
					struct device_driver driver;//��������
					const struct i2c_device_id *id_table;//i2c������֧�ֵ��豸�б�
				};
				
				struct device_driver {
					const char		*name;//����
					struct module		*owner;
					const struct of_device_id	*of_match_table;//�豸��ƥ��
				};
				struct of_device_id
				{
					char	compatible[128];
				};
				
				struct i2c_device_id {
					char name[I2C_NAME_SIZE];
				};
				
		����API:
			��1��i2c_add_driver��struct i2c_driver *driver��
				���ܣ�ע��i2c_driver
				������driver:struct i2c_driver��ָ�����
				����ֵ���ɹ���0  ʧ�ܣ�-ERRNO
				
			��2��void i2c_del_driver(struct i2c_driver *driver)
				���ܣ�ע��i2c_driver
				������driver:struct i2c_driver��ָ�����
				����ֵ����
				
				
		i2c�豸����:
		/*
			��Ҫ��i2c���豸��Ϣ���豸������ʽȥ�ṩ���ο��ں��й���i2c�豸���豸��ȥд��
				i2c_7: i2c@138D0000{
					#address-cells=<1>;
					#size-cells=<0>;

					samsung,i2c-sda-delay=<100>;
					samsung,i2c-max-bus-freq=<20000>;
					pinctrl-0=<&i2c7_bus>;
					pinctrl-names="default";
					status="okay";
		 
						ov3640: sensor@3c{
							compatible = "samsung,OV3640";//��i2c_driver�е����豸����ƥ����
							reg = <0x3c>;//i2c�Ĵӻ���ַ
						}; 
				};			
		*/
			(1)�ṩ�豸��Ϣ��ԭ��ͼ + �����ֲ�
				mpu6050  + i2c5  + addr0x68
				
			(2)�ṩ�豸��Ϣ
				i2c_5: i2c@138B0000{
					#address-cells=<1>;
					#size-cells=<0>;

					samsung,i2c-sda-delay=<100>;
					samsung,i2c-max-bus-freq=<20000>;
					pinctrl-0=<&i2c5_bus>;
					pinctrl-names="default";
					status="okay";
		 
						mpu6050{
							compatible = "farsight,mpu6050";//��i2c_driver�е����豸����ƥ����
							reg = <0x68>;//i2c�Ĵӻ���ַ
						}; 
				};	
				
				 ��linux�ں˶���Ŀ¼ִ�У�make dtbs
				 �� xxx.dtb  ������ /tftpboot
			

4.�����д����
	��1��ģ����Ҫ��
	��2��struct i2c_driver�Ķ��� + ���
	��3��init ->ע��
				probe��
					��mpu6050����������ù���
					
				��Ϊ������Ҫ�ṩ��Ӧ�����ݣ�appҪ�����ײ��ṩ��
				    cdev���
	��4��exit ->ע��

 
 
 

 
 
 
 
 
 
 
 
 
 

		  