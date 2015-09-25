avconv -f alsa -ac 2 -ar 22050  -i pulse -f x11grab -r 30 -s 750x520 -i :0.0+0,0 out.flv
