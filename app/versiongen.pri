include(config.pri)

DEFINES += VERSION_MAJOR=\\\"$$VERSION_MAJOR\\\"
DEFINES += VERSION_MINOR=\\\"$$VERSION_MINOR\\\"
DEFINES += VERSION_PATCH=\\\"$$VERSION_PATCH\\\"
DEFINES += BUILD_DATE=$$system(date +%s)
#win32 {
#    GIT_PATH = $$[GIT_PATH]
#    isEmpty(GIT_PATH): GIT_PATH = $$system_path("C:/Program Files/Git/bin/git.exe")
#} else {
    GIT_PATH=git
#}

GIT_BIN = $$system_quote($$GIT_PATH)

defineReplace(callGit){
    #it could be usefull when building from "external" scripting (not qmake).
    #repoDir = --git-dir $$system_path($${topSrcdir}/.git)
    #args = $$GIT_BIN $$repoDir $$1

    args = $$GIT_BIN $$1
    res = $$system($$args)
    return($$res)
}

#!CONFIG(debug, debug|release):CONFIG(release) {

    GIT_REV_INFO_branch = $$callGit("rev-parse --abbrev-ref HEAD")

    # If the branch name contains the number sign (#), it's treated as a comment by qmake:
    # GIT_REV_INFO_branch = branch_for_issue_#123 # it's evaluated to "branch_for_issue_"
    # GIT_REV_INFO = abc123@branch_for_issue_#123* # it's evaluated to "abc123@branch_for_issue_"
    # Being passed to the compiler, the define's body has no trailing quote:
    # cl -c -nologo <…> -DGIT_REV_INFO=\"abc123@branch_for_issue_ -I<…>
    GIT_REV_INFO_branch = $$replace(GIT_REV_INFO_branch, $${LITERAL_HASH}, "No")

    GIT_REV_INFO_commit = $$callGit("rev-parse --short HEAD") #full: git rev-parse HEAD or  git log -1 --pretty=format:%h
    dirty = $$callGit("diff-index --name-only HEAD")
    isEmpty(dirty) {
        GIT_REV_INFO_state=""
    } else {
        GIT_REV_INFO_state="*" #dirty
    }

    GIT_REV_INFO = $$sprintf("%1@%2%3", $$GIT_REV_INFO_commit, $$GIT_REV_INFO_branch, $$GIT_REV_INFO_state)

    DEFINES += GIT_REV_INFO=\\\"$$GIT_REV_INFO\\\"

    export (GIT_REV_INFO)
#}
