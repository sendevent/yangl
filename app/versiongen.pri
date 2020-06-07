shellScript=$$system_quote("$$_PRO_FILE_PWD_/../version.sh")

DEFINES += VERSION_MAJOR=\\\"$$system("$$shellScript a")\\\"
DEFINES += VERSION_MINOR=\\\"$$system("$$shellScript b")\\\"
DEFINES += VERSION_PATCH=\\\"$$system("$$shellScript c")\\\"

# If the branch name contains the number sign (#), it's treated as a comment by qmake:
# GIT_REV_INFO_branch = branch_for_issue_#123 # it's evaluated to "branch_for_issue_"
# GIT_REV_INFO = abc123@branch_for_issue_#123* # it's evaluated to "abc123@branch_for_issue_"
# Being passed to the compiler, the define's body has no trailing quote:
# cl -c -nologo <…> -DGIT_REV_INFO=\"abc123@branch_for_issue_ -I<…>
GIT_REV_INFO_branch = $$system("$$shellScript d")
GIT_REV_INFO_branch = $$replace(GIT_REV_INFO_branch, $${LITERAL_HASH}, "No")
DEFINES += GIT_REV_INFO=\\\"$$GIT_REV_INFO_branch\\\"

DEFINES += BUILD_DATE=$$system(date +%s)
