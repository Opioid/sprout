((nil . (
		 (eval . (set (make-local-variable 'my-project-path)
                      (file-name-directory
                       (let ((d (dir-locals-find-file ".")))
                         (if (stringp d) d (car d))))))
         (eval . (setq-local company-clang-arguments
							 (list
							  (concat "-std=c++14")
							  (concat "-I" my-project-path)
							  (concat "-I" my-project-path "base")
							  (concat "-I" my-project-path "core")
							  (concat "-I" my-project-path "extension")
							  (concat "-I" my-project-path "cli")
							  )
							 ))
		 (eval . (setq-local ff-search-directories
							 (list
							  (concat ".")
							  (concat my-project-path)
							  (concat my-project-path "base")
							  (concat my-project-path "core")
							  (concat my-project-path "extension")
							  (concat my-project-path "cli")
							  )
							 ))
         )))
