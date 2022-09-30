docker run\
 --rm \
 -v "$(realpath ..)":/home/devel/lsdnet\
 --name lsdnet_web_vis\
 --runtime nvidia\
 --workdir /home/devel/lsdnet/visualization/\
 -p 7860:7860\
 -t lsdnet\
 python display_gradio.py
