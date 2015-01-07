;; -*- lisp -*-
;;; kroute-mode.el -- major mode for editing kroute configuration file.

;; Copyright (C) 1998 Kunihiro Ishiguro

;; Author:     1998 Kunihiro Ishiguro
;;                  SeonMeyong HEO
;; Maintainer: kunihiro@kroute.org
;;             seirios@Matrix.IRI.Co.JP
;; Created:    Jan 28 1998
;; Version:    Alpha 0.2
;; Keywords:   kroute bgpd ripd ripngd languages

;; You can get the latest version of kroute from
;;
;;    http://www.kroute.org/
;;
;; Install this Emacs Lisp code
;;
;; Compile kroute.el
;;   % $(EMACS) -batch -f batch-byte-compile kroute.el
;; Install kroute.el,kroute.elc to Emacs-load-path
;;   % cp kroute.el kroute.elc $(emacs-load-path)
;; Add .emacs or (site-load.el | site-start.el)
;;   (auto-load 'kroute-mode "kroute" nil t)
;;   (auto-load 'bgp-mode "kroute" nil t)
;;   (auto-load 'rip-mode "kroute" nil t)
;;

;;; Code:

;; Set keywords

(defvar kroute-font-lock-keywords
  (list
   '("#.*$" . font-lock-comment-face)
   '("!.*$" . font-lock-comment-face)
   '("no\\|interface" . font-lock-type-face)
   '("ip6\\|ip\\|route\\|address" . font-lock-function-name-face)
   '("ipforward\\|ipv6forward" . font-lock-keyword-face)
   '("hostname\\|password\\|enable\\|logfile\\|no" . font-lock-keyword-face))
  "Default value to highlight in kroute mode.")

(defvar bgp-font-lock-keywords
  (list
   '("#.*$" . font-lock-comment-face)
   '("!.*$" . font-lock-comment-face)
   '("no\\|router" . font-lock-type-face)
   '("bgp\\|router-id\\|neighbor\\|network" . font-lock-function-name-face)
   '("ebgp\\|multihop\\|next\\|kroute\\|remote-as" . font-lock-keyword-face)
   '("hostname\\|password\\|enable\\|logfile\\|no" . font-lock-keyword-face))
  "Default value to highlight in bgp mode.")

(defvar rip-font-lock-keywords
  (list
   '("#.*$" . font-lock-comment-face)
   '("!.*$" . font-lock-comment-face)
   '("no\\|router\\|interface\\|ipv6\\|ip6\\|ip" . font-lock-type-face)
   '("ripng\\|rip\\|recive\\|advertize\\|accept" . font-lock-function-name-face)
   '("version\\|network" . font-lock-function-name-face)
   '("default\\|none\\|kroute" . font-lock-keyword-face)
   '("hostname\\|password\\|enable\\|logfile\\|no" . font-lock-keyword-face))
  "Default value to highlight in bgp mode.")

;; set font-lock-mode

(defun kroute-font-lock ()
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(kroute-font-lock-keywords nil t)))

(defun bgp-font-lock ()
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(bgp-font-lock-keywords nil t)))

(defun rip-font-lock ()
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(rip-font-lock-keywords nil t)))

;; define Major mode

(defun major-mode-define ()
  (interactive)
  (progn
    (setq comment-start "[#!]"
	  comment-end ""
	  comment-start-skip "!+ ")
    (run-hooks 'kroute-mode-hook)
    (cond
     ((string< "20" emacs-version)
      (font-lock-mode)))))

(defun kroute-mode ()
  (progn
    (setq mode-name "kroute")
    (kroute-font-lock))
  (major-mode-define))

(defun bgp-mode ()
  (progn
    (setq mode-name "bgp") 
    (bgp-font-lock))
  (major-mode-define))

(defun rip-mode ()
  (progn
    (setq mode-name "rip")
    (rip-font-lock))
  (major-mode-define))
