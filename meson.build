project('Practica 2 de PAV - deteccio d activitat vocal', 'c')

src = ['src/main_vad.c', 'src/vad.c','src/pav_analysis.c']
lib = ['-lm', '-lsndfile']
exe = 'vad'

executable(exe, sources: src, link_args: lib)