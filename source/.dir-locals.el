((nil . ((eval . (set (make-local-variable 'my-project-path)
                      (file-name-directory
                       (let ((d (dir-locals-find-file ".")))
                         (if (stringp d) d (car d))))))
         (eval . (setq-local company-clang-arguments
							 (list
							  (concat "-std=c++14")
							  (concat "-I" my-project-path)
							  (concat "-I" my-project-path "base/")
							  (concat "-I" my-project-path "core/")
							  (concat "-I" my-project-path "extension/")
							  )
							 )
			   )
         )))
