clear
%1.��ʼ�����趨ģ��
%.�������ڵ��������(��λ M)
xm=100;
ym=100;
%(1)��۽��������
sink.x=0.5*xm;
sink.y=0.5*ym;
%�����ڴ�������
n=100
%��ͷ�Ż���������ѡ��ͷ�ĸ��ʣ�
p=0.1;
%����ģ�ͣ���λ ����
%��ʼ������ģ��
Eo=0.5;
%Eelec=Etx=Erx
ETX=50*0.000000001;
ERX=50*0.000000001;
%Transmit Amplifier types
Efs=10*0.000000000001;
Emp=0.0013*0.000000000001;
%Data Aggregation Energy
EDA=5*0.000000001;
%�������ڵ㳬��һ�ڵ������İٷֱ�
a=1;
%���ѭ������
rmax=5000
%������� do
do=sqrt(Efs/Emp);
%2.���ߴ���������ģ�Ͳ���ģ��
%�������ߴ���������,�������ھ���Ͷ��100���ڵ�,������ͼ��
for i=1:1:n
    S1(i).xd=rand(1,1)*xm;
    S2(i).xd=S1(i).xd;
    XR2(i)=S2(i).xd;
    XR1(i)=S1(i).xd;
    S1(i).yd=rand(1,1)*ym;
    S2(i).yd=S1(i).yd;
    YR2(i)=S2(i).yd;
    YR1(i)=S1(i).yd;
    S1(i).G=0;
    S2(i).G=0;
    S1(i).E=Eo*(1+rand*a);
    S2(i).E=S1(i).E;
    %initially there are no cluster heads only nodes
    S1(i).type='N';
    S2(i).type='N';
end

S1(n+1).xd=sink.x;
S1(n+1).yd=sink.y;
S2(n+1).xd=sink.x;
S2(n+1).yd=sink.y;
%3.��������ģ��
%��ͷ�ڵ���
countCHs1=0;
cluster1=1;%�˶����Ŀ�Ľ����Ǹ���һ��1��ʼ���±�����������Ĵ�ͷ��Ӧ�û���ȥ1
flag_first_dead1=0;
flag_teenth_dead1=0;
flag_all_dead1=0;
%�����ڵ���
dead1=0;
first_dead1=0;
teenth_dead1=0;
all_dead1=0;
%��ڵ���
allive1=n;
%counter for bit transmitted to Bases Station and to Cluster Heads
packets_TO_BS1=0;
packets_TO_CH1=0;
%(1)ѭ��ģʽ�趨
for r=0:1:rmax     %�� for ѭ������������г���������ڣ�ֱ�����һ end �Ž���ѭ��
    r
  %ÿ��һ����ת����(������Ϊ10��)ʹ���ڵ��S��i��.G�������ò������ں���Ĵ�ѡ�٣��ڸ���ת�������ѵ�ѡ����ͷ�Ľڵ㲻���ٵ�ѡ���ָ�Ϊ��
  if(mod(r, round(1/p) )==0)
    for i=1:1:n
        S1(i).G=0;
        S1(i).cl=0;
    end
  end
%(2)�����ڵ���ģ��
dead1=0;
for i=1:1:n
    %������������ڵ�
    if (S1(i).E<=0)
        dead1=dead1+1; 
        %(3)��һ�������ڵ�Ĳ���ʱ��(���ִα�ʾ)
        %��һ���ڵ�����ʱ��
        if (dead1==1)
           if(flag_first_dead1==0)
              first_dead1=r;
              flag_first_dead1=1;
           end
        end
        %10%�Ľڵ�����ʱ��
        if(dead1==0.1*n)
           if(flag_teenth_dead1==0)
              teenth_dead1=r;
              flag_teenth_dead1=1;
           end
        end
        if(dead1==n)
           if(flag_all_dead1==0)
              all_dead1=r;
              flag_all_dead1=1;
           end
        end
    end
    if S1(i).E>0
        S1(i).type='N';
    end
end
STATISTICS.DEAD1(r+1)=dead1;
STATISTICS.ALLIVE1(r+1)=allive1-dead1;
%(4)��ͷѡ��ģ��
countCHs1=0;
cluster1=1;
for i=1:1:n
   if(S1(i).E>0)
   temp_rand=rand;     
   if ( (S1(i).G)<=0)  
       %��ͷ��ѡ�٣���ѡ�Ĵ�ͷ��Ѹ�������Ŵ�����������������ı�����
        if(temp_rand<= (p/(1-p*mod(r,round(1/p)))))
            countCHs1=countCHs1+1;
            packets_TO_BS1=packets_TO_BS1+1;
            PACKETS_TO_BS1(r+1)=packets_TO_BS1;
             S1(i).type='C';
            S1(i).G=round(1/p)-1;
            C1(cluster1).xd=S1(i).xd;
            C1(cluster1).yd=S1(i).yd;
           distance=sqrt( (S1(i).xd-(S1(n+1).xd) )^2 + (S1(i).yd-(S1(n+1).yd) )^2 );
            C1(cluster1).distance=distance;
            C1(cluster1).id=i;
            X1(cluster1)=S1(i).xd;
            Y1(cluster1)=S1(i).yd;
            cluster1=cluster1+1;
           %�����ͷ����4000bit���ݵ���վ���������ģ�����Ӧ�����нڵ������ͷÿһ�ַ���4000bit���ݣ�
           distance;
            if (distance>do)
                S1(i).E=S1(i).E- ( (ETX+EDA)*(4000) + Emp*4000*( distance*distance*distance*distance )); 
            end
            if (distance<=do)
                S1(i).E=S1(i).E- ( (ETX+EDA)*(4000)  + Efs*4000*( distance * distance )); 
            end
        end     
    
    end
    % S(i).G=S(i).G-1;  
   
 end 
end
STATISTICS.COUNTCHS1(r+1)=countCHs1;
%(5)���ڳ�Աѡ���ͷģ��(���ص��γ�ģ��)
%���ڳ�Ա�Դ�ͷ��ѡ�񣨼��ص��γɣ��㷨
for i=1:1:n
   if ( S1(i).type=='N' && S1(i).E>0 )
     if(cluster1-1>=1)
       min_dis=Inf;
       min_dis_cluster=0;
       for c=1:1:cluster1-1
           temp=min(min_dis,sqrt( (S1(i).xd-C1(c).xd)^2 + (S1(i).yd-C1(c).yd)^2 ) );
           if ( temp<min_dis )
               min_dis=temp;
               min_dis_cluster=c;
           end
       end
       %���ڽڵ㣨����4000bit���ݣ���������
       
            min_dis;
            if (min_dis>do)
                S1(i).E=S1(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S1(i).E=S1(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
        %��ͷ�����ܺ��ں���һ���ڽڵ�4000bit���ݣ�����������
            S1(C1(min_dis_cluster).id).E = S1(C1(min_dis_cluster).id).E- ( (ERX + EDA)*4000 ); 
            packets_TO_CH1=packets_TO_CH1+1;    
 
        S1(i).min_dis=min_dis;
        S1(i).min_dis_cluster=min_dis_cluster;
    else
        min_dis=sqrt( (S1(i).xd-S1(n+1).xd)^2 + (S1(i).yd-S1(n+1).yd)^2 );
            if (min_dis>do)
                S1(i).E=S1(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S1(i).E=S1(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS1=packets_TO_BS1+1;
     end
  end
end
STATISTICS.PACKETS_TO_CH1(r+1)=packets_TO_CH1;
STATISTICS.PACKETS_TO_BS1(r+1)=packets_TO_BS1;
end



%3.��������ģ��
%��ͷ�ڵ���
countCHs2=0;
cluster=1;%�˶����Ŀ�Ľ����Ǹ���һ��1��ʼ���±�����������Ĵ�ͷ��Ӧ�û���ȥ1
flag_first_dead2=0;
flag_teenth_dead2=0;
flag_all_dead2=0;
%�����ڵ���
dead2=0;
first_dead2=0;
teenth_dead2=0;
all_dead2=0;
%��ڵ���
allive2=n;
%counter for bit transmitted to Bases Station and to Cluster Heads
packets_TO_BS2=0;
packets_TO_CH2=0;
%(1)ѭ��ģʽ�趨
for r=0:1:rmax     %�� for ѭ������������г���������ڣ�ֱ�����һ end �Ž���ѭ��
    r
  %ÿ��һ����ת����(������Ϊ10��)ʹ���ڵ��S��i��.G�������ò������ں���Ĵ�ѡ�٣��ڸ���ת�������ѵ�ѡ����ͷ�Ľڵ㲻���ٵ�ѡ���ָ�Ϊ��
  if(mod(r, round(1/p) )==0)
    for i=1:1:n
        S2(i).G=0;
        S2(i).cl=0;
    end
  end
%(2)�����ڵ���ģ��
dead2=0;
for i=1:1:n
    %������������ڵ�
    if (S2(i).E<=0)
        dead2=dead2+1; 
        %(3)��һ�������ڵ�Ĳ���ʱ��(���ִα�ʾ)
        %��һ���ڵ�����ʱ��
        if (dead2==1)
           if(flag_first_dead2==0)
              first_dead2=r;
              flag_first_dead2=1;
           end
        end
        %10%�Ľڵ�����ʱ��
        if(dead2==0.1*n)
           if(flag_teenth_dead2==0)
              teenth_dead2=r;
              flag_teenth_dead2=1;
           end
        end
        if(dead2==n)
           if(flag_all_dead2==0)
              all_dead2=r;
              flag_all_dead2=1;
           end
        end
    end
    if S2(i).E>0
        S2(i).type='N';
    end
end
STATISTICS.DEAD2(r+1)=dead2;
STATISTICS.ALLIVE2(r+1)=allive2-dead2;
%(4)��ͷѡ��ģ��
countCHs2=0;
cluster2=1;
for i=1:1:n
   if(S2(i).E>0)
   temp_rand=rand;     
   if ( (S2(i).G)<=0)  
       %��ͷ��ѡ�٣���ѡ�Ĵ�ͷ��Ѹ�������Ŵ�����������������ı�����
        if(temp_rand<= (p/(1-p*mod(r,round(1/p)))))
            countCHs2=countCHs2+1;
            packets_TO_BS2=packets_TO_BS2+1;
            PACKETS_TO_BS2(r+1)=packets_TO_BS2;
             S2(i).type='C';
            S2(i).G=round(1/p)-1;
            C2(cluster2).xd=S2(i).xd;
            C2(cluster2).yd=S2(i).yd;
           distance=sqrt( (S2(i).xd-(S2(n+1).xd) )^2 + (S2(i).yd-(S2(n+1).yd) )^2 );
            C2(cluster2).distance=distance;
            C2(cluster2).id=i;
            X2(cluster2)=S2(i).xd;
            Y2(cluster2)=S2(i).yd;
            cluster2=cluster2+1;
           %�����ͷ����4000bit���ݵ���վ���������ģ�����Ӧ�����нڵ������ͷÿһ�ַ���4000bit���ݣ�
           distance;
            if (distance>do)
                S2(i).E=S2(i).E- ( (ETX+EDA)*(4000) + Emp*4000*( distance*distance*distance*distance )); 
            end
            if (distance<=do)
                S2(i).E=S2(i).E- ( (ETX+EDA)*(4000)  + Efs*4000*( distance * distance )); 
            end
        end     
    
    end
    % S2(i).G=S2(i).G-1;  
   
 end 
end
STATISTICS.COUNTCHS2(r+1)=countCHs2;
%(5)���ڳ�Աѡ���ͷģ��(���ص��γ�ģ��)
%���ڳ�Ա�Դ�ͷ��ѡ�񣨼��ص��γɣ��㷨
for i=1:1:n
   if ( S2(i).type=='N' && S2(i).E>0 )
     if(cluster2-1>=1)
       min_dis=sqrt( (S2(i).xd-S2(n+1).xd)^2 + (S2(i).yd-S2(n+1).yd)^2 );
       min_dis_cluster=0;
       for c=1:1:cluster2-1
           temp=min(min_dis,sqrt( (S2(i).xd-C2(c).xd)^2 + (S2(i).yd-C2(c).yd)^2 ) );
           if ( temp<min_dis )
               min_dis=temp;
               min_dis_cluster=c;
           end
       end
       %���ڽڵ㣨����4000bit���ݣ���������
       if(min_dis_cluster~=0)    
            min_dis;
            if (min_dis>do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
        %��ͷ�����ܺ��ں���һ���ڽڵ�4000bit���ݣ�����������
            S2(C2(min_dis_cluster).id).E = S2(C2(min_dis_cluster).id).E- ( (ERX + EDA)*4000 ); 
            packets_TO_CH2=packets_TO_CH2+1;
       else 
            min_dis;
            if (min_dis>do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS2=packets_TO_BS2+1;
            
       end
        S2(i).min_dis=min_dis;
        S2(i).min_dis_cluster=min_dis_cluster;
    else
        min_dis=sqrt( (S2(i).xd-S2(n+1).xd)^2 + (S2(i).yd-S2(n+1).yd)^2 );
            if (min_dis>do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S2(i).E=S2(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS2=packets_TO_BS2+1;
     end
  end
end
STATISTICS.PACKETS_TO_CH2(r+1)=packets_TO_CH2;
STATISTICS.PACKETS_TO_BS2(r+1)=packets_TO_BS2;
end
r=0:5000;
subplot(1,2,1);
plot(r,STATISTICS.DEAD1,'--k',r,STATISTICS.DEAD2,'-r');
legend('leachm1','leachm2');
xlabel('x(time)');
ylabel('y(dead)');
title('\bf leachm1��leachm2���������ڶԱ�');
subplot(1,2,2);
plot(r,STATISTICS.PACKETS_TO_BS1,'--k',r,STATISTICS.PACKETS_TO_BS2,'-r');
legend('leachm1','leachm2');
xlabel('x(time)');
ylabel('y(data)');
title('\bf leachsm��leachm2�����ݴ���Ա�');