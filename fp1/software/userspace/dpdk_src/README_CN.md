### *`��Щ����������������Ը��û��ķ�ʽ����`* ###

[Switch to the English version](./README.md)

*`�ļ��б�`*

* `dpdk-16.04.tar.bz2`: DPDKԴ����
* `securec.tar.bz2`: ��ȫ������Դ����
* `build_dpdk.sh`: DPDK����ű�

# 1. ��ѹ��ȫ������Դ���

`tar -xjv -f securec.tar.bz2`  

# 2. ���밲ȫ������

`cd securec/`  
`sh ./securec_make.sh`  

# 3. ��ѹDPDKԴ���

`cd ..`  
`tar -xjv -f dpdk-16.04.tar.bz2`

# 4. ����DPDK

`cd dpdk-16.04`  
`make config T=x86_64-native-linuxapp-gcc`  
`make`  
`make install T=x86_64-native-linuxapp-gcc`  

### ��ʾ:  
���Ĳ���ɺ���ӡ"Installation cannot run with T defined and DESTDIR undefined"������ʾ���Ǵ���

�ɹ�ִ�к��ڵ�ǰĿ¼�����һ��`x86_64-native-linuxapp-gcc`�ļ��У����а���DPDK��ͷ�ļ�Ŀ¼�Ϳ�Ŀ¼��