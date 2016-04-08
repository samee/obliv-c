.PHONY: all clean

DEPENDS=$(shell ls *.tex)
NAME=mpc_poster
all: $(NAME).pdf

$(NAME).pdf: $(NAME).tex $(DEPENDS)
	rubber -f --pdf --warn all $<

fast:
	pkill Preview || true && pdflatex $(NAME).tex && bibtex $(NAME) && pdflatex $(NAME).tex & open $(NAME).pdf

open:
	pkill Preview || true && open $(NAME).pdf

clean:
	rm -rf *.aux *.bbl *.blg *.log *.pdf *.toc *.snm *.out *.nav tags auto
