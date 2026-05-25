#include <cstring>

#include "constants.h"
#include "given.h"
#include "pa3_task.h"
#include "structures.h"

using namespace std;

/**
 * Task 1 - Register New User
 *
 * The `register_new_user` function registers a new user on the version control
 * system if no existing user has the specified username already.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param username: the username of the new user to create.
 * @returns: `nullptr` if any validation failed; a pointer to the newly created
 *           user otherwise.
 */
const User *register_new_user(UserManagement &userManagement,
                              const char *username)
{
  // TODO: Task 1
  if (strlen(username) >= MAX_USER_NAME_LEN)
  {
    cout << "Username length exceeded " << MAX_USER_NAME_LEN - 1 << " characters." << endl;
    return nullptr;
  }

  User *curr = userManagement.head;
  User *prev = nullptr;

  while (curr != nullptr)
  {
    int cmp = strcmp(curr->name, username);
    if (cmp == 0)
    {
      cout << "User " << username << " already exists." << endl;
      return nullptr;
    }
    if (cmp > 0)
      break;
    prev = curr;
    curr = curr->next;
  }

  User *newUser = new User;
  strcpy(newUser->name, username);
  newUser->numRepos = 0;
  newUser->repos = nullptr;
  newUser->next = curr;

  if (prev == nullptr)
  {
    userManagement.head = newUser;
  }
  else
  {
    prev->next = newUser;
  }

  return newUser;
}

/**
 * Task 2 - Create Repository
 *
 * The `create_repository` function creates a repository with the specified name
 * under the specified owner, if no existing repository under the very user has
 * the same name as specified.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param owner: the owner of the new repository.
 * @param repoName: the name of the new repository.
 * @param creationTimestamp: the timestamp for the creation of the repository,
 *                           used for the initial commit.
 * @returns: -1 if any validation fails before repository creation; the index
 *           into the array of pointers to repositories under the owner user
 *           associated with this new repository otherwise.
 */
int create_repository(RepositoryManagement &repoManagement, User *owner,
                      const char *repoName, const time_t creationTimestamp)
{
  // TODO: Task 2
  if (strlen(repoName) >= MAX_REPO_NAME_LEN)
  {
    cout << "Repository name length exceeded " << MAX_REPO_NAME_LEN - 1 << " characters." << endl;
    return -1;
  }

  for (int i = 0; i < owner->numRepos; ++i)
  {
    if (strcmp(owner->repos[i]->name, repoName) == 0)
    {
      cout << "Repository " << owner->name << "/" << repoName << " already exists." << endl;
      return -1;
    }
  }

  Repository *newRepo = new Repository;
  newRepo->owner = owner;
  strcpy(newRepo->name, repoName);
  newRepo->numPrs = 0;
  newRepo->prs = nullptr;
  newRepo->numForks = 0;
  newRepo->forks = nullptr;
  newRepo->numBranches = 0;
  newRepo->branches = nullptr;

  Commit *initCommit = new Commit;
  initCommit->author = owner;
  strcpy(initCommit->message, "Initial commit.");
  initCommit->timestamp = creationTimestamp;
  initCommit->next = initCommit->prev = nullptr;

  SHA1 hasher;
  initialize(hasher);
  input(hasher, owner->name, strlen(owner->name));
  input(hasher, initCommit->message, strlen(initCommit->message));
  input(hasher, creationTimestamp);
  digest(hasher);
  initCommit->hash = hasher;
  newRepo->commits = initCommit;

  Repository **newOwnerRepos = new Repository *[owner->numRepos + 1];
  int insertPos = 0;
  while (insertPos < owner->numRepos && strcmp(owner->repos[insertPos]->name, repoName) < 0)
  {
    newOwnerRepos[insertPos] = owner->repos[insertPos];
    insertPos++;
  }
  newOwnerRepos[insertPos] = newRepo;
  for (int i = insertPos; i < owner->numRepos; ++i)
  {
    newOwnerRepos[i + 1] = owner->repos[i];
  }
  delete[] owner->repos;
  owner->repos = newOwnerRepos;
  owner->numRepos++;

  Repository **newGlobalRepos = new Repository *[repoManagement.numRepos + 1];
  int gPos = 0;
  while (gPos < repoManagement.numRepos)
  {
    int ownerCmp = strcmp(repoManagement.repos[gPos]->owner->name, owner->name);
    if (ownerCmp > 0 || (ownerCmp == 0 && strcmp(repoManagement.repos[gPos]->name, repoName) > 0))
      break;
    newGlobalRepos[gPos] = repoManagement.repos[gPos];
    gPos++;
  }
  newGlobalRepos[gPos] = newRepo;
  for (int i = gPos; i < repoManagement.numRepos; ++i)
  {
    newGlobalRepos[i + 1] = repoManagement.repos[i];
  }
  delete[] repoManagement.repos;
  repoManagement.repos = newGlobalRepos;
  repoManagement.numRepos++;

  return gPos;
}

/**
 * Task 3 - Create Branch
 *
 * The `create_branch` function creates a branch in the specified repository
 * with the supplied name and creator of the branch, at the specified commit,
 * if no existing branch has the same name already.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the fully-qualified name of the repository to create a branch
 *                 for.
 * @param branchName: the name of the branch to create.
 * @param creator: pointer to the creator user of the branch
 * @param commit: the commit to create the branch from.
 * @returns: false if any validation failed; true if the branch was created
 *           successfully.
 */
bool create_branch(RepositoryManagement &repoManagement, char *repoFQN,
                   const char *branchName, const User *creator,
                   const Commit *commit)
{
  // TODO: Task 3
  Repository *repo = nullptr;
  char *slash = strchr(repoFQN, '/');
  if (!slash)
    return false;

  *slash = '\0';
  char *ownerName = repoFQN;
  char *rName = slash + 1;

  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, ownerName) == 0 &&
        strcmp(repoManagement.repos[i]->name, rName) == 0)
    {
      repo = repoManagement.repos[i];
      break;
    }
  }
  *slash = '/';

  if (!repo)
  {
    cout << "Repository " << repoFQN << " does not exist." << endl;
    return false;
  }
  if (strlen(branchName) >= MAX_BRANCH_NAME_LEN)
  {
    cout << "Branch name length exceeded " << MAX_BRANCH_NAME_LEN - 1 << " characters." << endl;
    return false;
  }
  for (int i = 0; i < repo->numBranches; ++i)
  {
    if (strcmp(repo->branches[i]->name, branchName) == 0)
    {
      cout << "Branch " << repoFQN << ":" << branchName << " already exists." << endl;
      return false;
    }
  }

  Branch *newBranch = new Branch;
  strcpy(newBranch->name, branchName);
  newBranch->creator = const_cast<User *>(creator);
  newBranch->repo = repo;
  newBranch->head = commit ? const_cast<Commit *>(commit) : repo->commits;

  Branch **newBranches = new Branch *[repo->numBranches + 1];
  int pos = 0;
  while (pos < repo->numBranches && strcmp(repo->branches[pos]->name, branchName) < 0)
  {
    newBranches[pos] = repo->branches[pos];
    pos++;
  }
  newBranches[pos] = newBranch;
  for (int i = pos; i < repo->numBranches; ++i)
  {
    newBranches[i + 1] = repo->branches[i];
  }
  delete[] repo->branches;
  repo->branches = newBranches;
  repo->numBranches++;

  return true;
}

/**
 * Task 4 - Add Commit
 *
 * The `add_commit` function adds a commit in the specified repository
 * with an optionally-specified branch to add the commit to. The hash
 * of the commit is computed from the author and message of the current
 * commit, as well as those of the previous commit (if any).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param author: the author of the commit.
 * @param repoFQN: fully-qualified name of the repository to add the commit
 *                 to.
 * @param commitMessage: the message of the commit.
 * @param branch: optionally, the branch the commit is added to.
 * @param timestamp: the timestamp when the commit was created.
 */
void add_commit(RepositoryManagement &repoManagement, const User *author,
                char *repoFQN, const char *branch, const char *commitMessage,
                time_t timestamp)
{
  // TODO: Task 4
  Repository *repo = nullptr;
  char tempFQN[256];
  strcpy(tempFQN, repoFQN);
  char *ownerN = strtok(tempFQN, "/");
  char *repoN = strtok(nullptr, "/");

  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, ownerN) == 0 &&
        strcmp(repoManagement.repos[i]->name, repoN) == 0)
    {
      repo = repoManagement.repos[i];
      break;
    }
  }

  if (!repo)
  {
    cout << "Repository " << repoFQN << " does not exist." << endl;
    return;
  }

  Commit **headPtr = &repo->commits;
  if (branch && strcmp(branch, "main") != 0)
  {
    bool found = false;
    for (int i = 0; i < repo->numBranches; ++i)
    {
      if (strcmp(repo->branches[i]->name, branch) == 0)
      {
        headPtr = &repo->branches[i]->head;
        found = true;
        break;
      }
    }
    if (!found)
    {
      cout << "Branch " << repoFQN << ":" << branch << " does not exist." << endl;
      return;
    }
  }

  Commit *newCommit = new Commit;
  newCommit->author = const_cast<User *>(author);
  newCommit->timestamp = timestamp;

  if (strlen(commitMessage) >= MAX_COMMIT_MSG_LEN)
  {
    strncpy(newCommit->message, commitMessage, MAX_COMMIT_MSG_LEN - 1);
    newCommit->message[MAX_COMMIT_MSG_LEN - 1] = '\0';
    cout << "Warning: commit message is longer than " << MAX_COMMIT_MSG_LEN - 1
         << " characters and will be truncated." << endl;
  }
  else
  {
    strcpy(newCommit->message, commitMessage);
  }

  SHA1 hasher;
  initialize(hasher);
  input(hasher, author->name, strlen(author->name));
  input(hasher, newCommit->message, strlen(newCommit->message));
  input(hasher, timestamp);

  if (*headPtr)
  {
    input(hasher, (*headPtr)->author->name, strlen((*headPtr)->author->name));
    input(hasher, (*headPtr)->message, strlen((*headPtr)->message));
    input(hasher, (*headPtr)->timestamp);
  }
  digest(hasher);
  newCommit->hash = hasher;

  newCommit->next = *headPtr;
  newCommit->prev = nullptr;
  if (*headPtr)
    (*headPtr)->prev = newCommit;
  *headPtr = newCommit;

  cout << "Pushing commit ";
  print_sha(newCommit->hash);
  cout << " to branch " << repoFQN << ":" << (branch ? branch : "main") << "." << endl;
}

/**
 * Task 5 - Transfer Ownership
 *
 * The `transfer_ownership` function transfers the ownership of a repository
 * from its current owner to another user. Both users have to be registered
 * users on the platform.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param fromUsername: the name of the current owner of the specified
 *                      repsitory.
 * @param toUsername: the new owner of the specified repository.
 * @param repoName: the name of the repository.
 * @returns: true if the ownership transfer was successful; false otherwise.
 */
bool transfer_ownership(UserManagement &userManagement,
                        RepositoryManagement &repoManagement,
                        const char *fromUsername, const char *toUsername,
                        const char *repoName)
{
  // TODO: Task 5
  if (strcmp(fromUsername, toUsername) == 0)
  {
    cout << "Why are you transferring ownership to the same user?" << endl;
    return false;
  }

  User *fromUser = nullptr, *toUser = nullptr;
  User *curr = userManagement.head;
  while (curr)
  {
    if (strcmp(curr->name, fromUsername) == 0)
      fromUser = curr;
    if (strcmp(curr->name, toUsername) == 0)
      toUser = curr;
    curr = curr->next;
  }

  if (!fromUser)
  {
    cout << "User " << fromUsername << " does not exist." << endl;
    return false;
  }
  if (!toUser)
  {
    cout << "User " << toUsername << " does not exist." << endl;
    return false;
  }

  int repoIdx = -1;
  for (int i = 0; i < fromUser->numRepos; ++i)
  {
    if (strcmp(fromUser->repos[i]->name, repoName) == 0)
    {
      repoIdx = i;
      break;
    }
  }
  if (repoIdx == -1)
  {
    cout << "Repository " << fromUsername << "/" << repoName << " not found." << endl;
    return false;
  }

  for (int i = 0; i < toUser->numRepos; ++i)
  {
    if (strcmp(toUser->repos[i]->name, repoName) == 0)
    {
      cout << "Repository " << toUsername << "/" << repoName << " already exists." << endl;
      return false;
    }
  }

  Repository *repo = fromUser->repos[repoIdx];

  for (int i = repoIdx; i < fromUser->numRepos - 1; ++i)
    fromUser->repos[i] = fromUser->repos[i + 1];
  fromUser->numRepos--;

  repo->owner = toUser;

  Repository **newToRepos = new Repository *[toUser->numRepos + 1];
  int pos = 0;
  while (pos < toUser->numRepos && strcmp(toUser->repos[pos]->name, repoName) < 0)
  {
    newToRepos[pos] = toUser->repos[pos];
    pos++;
  }
  newToRepos[pos] = repo;
  for (int i = pos; i < toUser->numRepos; ++i)
  {
    newToRepos[i + 1] = toUser->repos[i];
  }
  delete[] toUser->repos;
  toUser->repos = newToRepos;
  toUser->numRepos++;

  int globalIdx = -1;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (repoManagement.repos[i] == repo)
    {
      globalIdx = i;
      break;
    }
  }
  for (int i = globalIdx; i < repoManagement.numRepos - 1; ++i)
    repoManagement.repos[i] = repoManagement.repos[i + 1];
  repoManagement.numRepos--;

  int newGlobalPos = 0;
  while (newGlobalPos < repoManagement.numRepos)
  {
    int cmp = strcmp(repoManagement.repos[newGlobalPos]->owner->name, toUser->name);
    if (cmp > 0 || (cmp == 0 && strcmp(repoManagement.repos[newGlobalPos]->name, repoName) > 0))
      break;
    newGlobalPos++;
  }
  for (int i = repoManagement.numRepos; i > newGlobalPos; --i)
    repoManagement.repos[i] = repoManagement.repos[i - 1];
  repoManagement.repos[newGlobalPos] = repo;
  repoManagement.numRepos++;

  return true;
}

/**
 * Task 6 - Create Pull Request
 *
 * The `create_pull_request` function creates a pull request from one branch of
 * a repository to another. The repositories can be different.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param title: the creator of the pull request
 * @param author: the author of the pull request
 * @param fromBranchFQN: fully-qualified name of the branch to make a pul
 *                        request from.
 * @param toBranchFQN: fully-qualified name of the branch to merge the
 *                      suggested changes to.
 * @returns: true if the pull request was created successfully.
 */
bool create_pull_request(const RepositoryManagement &repoManagement,
                         const char *title, const User *author,
                         char *fromBranchFQN, char *toBranchFQN)
{
  // TODO: Task 6
  if (strlen(title) > MAX_PR_TITLE_LEN - 1)
  {
    cout << "Pull request title length exceeded " << MAX_PR_TITLE_LEN - 1 << " characters." << endl;
    return false;
  }

  auto parse_fqn = [](char *fqn, char *owner, char *repoName, char *branch) -> bool
  {
    char temp[256];
    strcpy(temp, fqn);
    char *ownerTok = strtok(temp, "/");
    char *repoPart = strtok(nullptr, ":");
    char *branchTok = strtok(nullptr, "");
    if (!ownerTok || !repoPart || !branchTok)
      return false;
    strcpy(owner, ownerTok);
    strcpy(repoName, repoPart);
    strcpy(branch, branchTok);
    return true;
  };

  char srcOwner[256], srcRepoName[256], srcBranchName[256];
  if (!parse_fqn(fromBranchFQN, srcOwner, srcRepoName, srcBranchName))
    return false;

  Repository *srcRepo = nullptr;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, srcOwner) == 0 &&
        strcmp(repoManagement.repos[i]->name, srcRepoName) == 0)
    {
      srcRepo = repoManagement.repos[i];
      break;
    }
  }
  if (!srcRepo)
  {
    cout << "Repository " << srcOwner << "/" << srcRepoName << " does not exist." << endl;
    return false;
  }

  Branch *srcBranch = nullptr;
  if (strcmp(srcBranchName, "main") == 0)
  {
    srcBranch = new Branch;
    strcpy(srcBranch->name, "main");
    srcBranch->creator = srcRepo->owner;
    srcBranch->head = srcRepo->commits;
    srcBranch->repo = srcRepo;
  }
  else
  {
    for (int i = 0; i < srcRepo->numBranches; ++i)
    {
      if (strcmp(srcRepo->branches[i]->name, srcBranchName) == 0)
      {
        srcBranch = srcRepo->branches[i];
        break;
      }
    }
    if (!srcBranch)
    {
      cout << "Branch " << fromBranchFQN << " does not exist." << endl;
      return false;
    }
  }

  char dstOwner[256], dstRepoName[256], dstBranchName[256];
  if (!parse_fqn(toBranchFQN, dstOwner, dstRepoName, dstBranchName))
    return false;

  Repository *dstRepo = nullptr;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, dstOwner) == 0 &&
        strcmp(repoManagement.repos[i]->name, dstRepoName) == 0)
    {
      dstRepo = repoManagement.repos[i];
      break;
    }
  }
  if (!dstRepo)
  {
    cout << "Repository " << dstOwner << "/" << dstRepoName << " does not exist." << endl;
    return false;
  }

  Branch *dstBranch = nullptr;
  if (strcmp(dstBranchName, "main") == 0)
  {
    dstBranch = nullptr;
  }
  else
  {
    for (int i = 0; i < dstRepo->numBranches; ++i)
    {
      if (strcmp(dstRepo->branches[i]->name, dstBranchName) == 0)
      {
        dstBranch = dstRepo->branches[i];
        break;
      }
    }
    if (!dstBranch)
    {
      cout << "Branch " << toBranchFQN << " does not exist." << endl;
      return false;
    }
  }

  PullRequest *newPr = new PullRequest;
  newPr->id = dstRepo->numPrs + 1;
  newPr->author = author;
  newPr->fromBranch = srcBranch;
  newPr->toBranch = dstBranch;
  newPr->repo = dstRepo;
  strcpy(newPr->title, title);
  newPr->status = OPEN;

  PullRequest **newPrs = new PullRequest *[dstRepo->numPrs + 1];
  for (int i = 0; i < dstRepo->numPrs; ++i)
    newPrs[i] = dstRepo->prs[i];
  newPrs[dstRepo->numPrs] = newPr;
  delete[] dstRepo->prs;
  dstRepo->prs = newPrs;
  dstRepo->numPrs++;

  cout << "Pull request #" << newPr->id << " has been created in "
       << dstRepo->owner->name << "/" << dstRepo->name << "." << endl;
  return true;
}

/**
 * Task 7 - Fork Repository
 *
 * The `fork_repository` function allows the creation of forks of repositories.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param owner: the owner of the repository to create the fork from.
 * @param forkedOwner: the ownr of the forked repository.
 * @param repoToFork: the name of the repository to fork.
 * @returns true if the repository was forked successfully; false otherwise.
 */
bool fork_repository(UserManagement &userManagement,
                     RepositoryManagement &repoManagement, const User *owner,
                     const char *forkedOwner, const char *repoToFork)
{
  // TODO: Task 7
  Repository *srcRepo = nullptr;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, owner->name) == 0 &&
        strcmp(repoManagement.repos[i]->name, repoToFork) == 0)
    {
      srcRepo = repoManagement.repos[i];
      break;
    }
  }
  if (!srcRepo)
  {
    cout << "Repository " << owner->name << "/" << repoToFork << " does not exist." << endl;
    return false;
  }

  User *forker = nullptr;
  User *curr = userManagement.head;
  while (curr)
  {
    if (strcmp(curr->name, forkedOwner) == 0)
    {
      forker = curr;
      break;
    }
    curr = curr->next;
  }
  if (!forker)
  {
    cout << "User " << forkedOwner << " does not exist." << endl;
    return false;
  }

  for (int i = 0; i < forker->numRepos; ++i)
  {
    if (strcmp(forker->repos[i]->name, repoToFork) == 0)
    {
      cout << "Repository " << forkedOwner << "/" << repoToFork << " already exists." << endl;
      return false;
    }
  }

  const int MAX_COMMITS = 1024;
  Commit *srcCommits[MAX_COMMITS];
  int srcCommitCount = 0;

  auto addCommit = [&](Commit *c)
  {
    if (!c)
      return;
    for (int i = 0; i < srcCommitCount; ++i)
      if (srcCommits[i] == c)
        return;
    srcCommits[srcCommitCount++] = c;
  };

  Commit *c = srcRepo->commits;
  while (c)
  {
    addCommit(c);
    c = c->next;
  }

  for (int i = 0; i < srcRepo->numBranches; ++i)
  {
    c = srcRepo->branches[i]->head;
    while (c)
    {
      addCommit(c);
      c = c->next;
    }
  }

  Commit *newCommits[MAX_COMMITS];
  for (int i = 0; i < srcCommitCount; ++i)
  {
    Commit *old = srcCommits[i];
    Commit *newC = new Commit;
    newC->author = old->author;
    strcpy(newC->message, old->message);
    newC->timestamp = old->timestamp;
    newC->hash = old->hash;
    newC->next = nullptr;
    newC->prev = nullptr;
    newCommits[i] = newC;
  }

  auto getNewCommit = [&](Commit *old) -> Commit *
  {
    for (int i = 0; i < srcCommitCount; ++i)
      if (srcCommits[i] == old)
        return newCommits[i];
    return nullptr;
  };

  Commit *newMainHead = nullptr;
  if (srcRepo->commits)
  {
    newMainHead = getNewCommit(srcRepo->commits);
    Commit *newPrev = nullptr;
    Commit *oldCurr = srcRepo->commits;
    while (oldCurr)
    {
      Commit *newCurr = getNewCommit(oldCurr);
      if (newCurr)
      {
        newCurr->prev = newPrev;
        if (newPrev)
          newPrev->next = newCurr;
        newPrev = newCurr;
      }
      oldCurr = oldCurr->next;
    }
  }

  for (int i = 0; i < srcCommitCount; ++i)
  {
    Commit *old = srcCommits[i];
    Commit *newC = newCommits[i];
    if (old->next)
      newC->next = getNewCommit(old->next);
    if (old->prev)
      newC->prev = getNewCommit(old->prev);
  }

  Repository *newRepo = new Repository;
  newRepo->owner = forker;
  strcpy(newRepo->name, repoToFork);
  newRepo->numPrs = 0;
  newRepo->prs = nullptr;
  newRepo->numForks = 0;
  newRepo->forks = nullptr;
  newRepo->commits = newMainHead;
  newRepo->numBranches = srcRepo->numBranches;
  newRepo->branches = new Branch *[newRepo->numBranches];

  for (int i = 0; i < srcRepo->numBranches; ++i)
  {
    Branch *oldBranch = srcRepo->branches[i];
    Branch *newBranch = new Branch;
    strcpy(newBranch->name, oldBranch->name);
    newBranch->creator = oldBranch->creator;
    newBranch->repo = newRepo;
    newBranch->head = getNewCommit(oldBranch->head);
    newRepo->branches[i] = newBranch;
  }

  for (int i = 0; i < newRepo->numBranches - 1; ++i)
  {
    for (int j = i + 1; j < newRepo->numBranches; ++j)
    {
      if (strcmp(newRepo->branches[i]->name, newRepo->branches[j]->name) > 0)
      {
        Branch *tmp = newRepo->branches[i];
        newRepo->branches[i] = newRepo->branches[j];
        newRepo->branches[j] = tmp;
      }
    }
  }

  Repository **newGlobalRepos = new Repository *[repoManagement.numRepos + 1];
  int gpos = 0;
  while (gpos < repoManagement.numRepos)
  {
    int cmpOwner = strcmp(repoManagement.repos[gpos]->owner->name, forkedOwner);
    if (cmpOwner > 0 ||
        (cmpOwner == 0 && strcmp(repoManagement.repos[gpos]->name, repoToFork) >= 0))
    {
      break;
    }
    newGlobalRepos[gpos] = repoManagement.repos[gpos];
    ++gpos;
  }
  newGlobalRepos[gpos] = newRepo;
  for (int i = gpos; i < repoManagement.numRepos; ++i)
  {
    newGlobalRepos[i + 1] = repoManagement.repos[i];
  }
  delete[] repoManagement.repos;
  repoManagement.repos = newGlobalRepos;
  repoManagement.numRepos++;

  Repository **newUserRepos = new Repository *[forker->numRepos + 1];
  int upos = 0;
  while (upos < forker->numRepos && strcmp(forker->repos[upos]->name, repoToFork) < 0)
  {
    newUserRepos[upos] = forker->repos[upos];
    ++upos;
  }
  newUserRepos[upos] = newRepo;
  for (int i = upos; i < forker->numRepos; ++i)
  {
    newUserRepos[i + 1] = forker->repos[i];
  }
  delete[] forker->repos;
  forker->repos = newUserRepos;
  forker->numRepos++;

  Repository **newForks = new Repository *[srcRepo->numForks + 1];
  int fpos = 0;
  while (fpos < srcRepo->numForks)
  {
    int cmpOwner = strcmp(srcRepo->forks[fpos]->owner->name, forkedOwner);
    if (cmpOwner > 0 ||
        (cmpOwner == 0 && strcmp(srcRepo->forks[fpos]->name, repoToFork) > 0))
    {
      break;
    }
    newForks[fpos] = srcRepo->forks[fpos];
    ++fpos;
  }
  newForks[fpos] = newRepo;
  for (int i = fpos; i < srcRepo->numForks; ++i)
  {
    newForks[i + 1] = srcRepo->forks[i];
  }
  delete[] srcRepo->forks;
  srcRepo->forks = newForks;
  srcRepo->numForks++;

  cout << "Fork " << forkedOwner << "/" << repoToFork << " created successfully." << endl;
  return true;
}

/**
 * Task 8.1 - Merge Pull Request (Squash Merge)
 *
 * The `merge_pull_request_squashmerge` function merges the specified
 * pull request in a repsitory using the squash merge strategy
 * (combines all commits in the pull request into one and add it to the target
 * branch).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_squashmerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp)
{
  // TODO: Task 8.1
  char fqnCopy[256];
  strcpy(fqnCopy, repoFQN);
  char *ownerName = strtok(fqnCopy, "/");
  char *repoNameFromFQN = strtok(nullptr, "/");
  if (!ownerName || !repoNameFromFQN)
    return;

  Repository *repo = nullptr;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, ownerName) == 0 &&
        strcmp(repoManagement.repos[i]->name, repoNameFromFQN) == 0)
    {
      repo = repoManagement.repos[i];
      break;
    }
  }
  if (!repo)
  {
    cout << "Repository " << repoFQN << " does not exist." << endl;
    return;
  }

  if (prNumber <= 0 || prNumber > repo->numPrs)
  {
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }

  PullRequest *pr = repo->prs[prNumber - 1];
  if (pr->status != OPEN)
    return;

  Commit *targetHead = pr->toBranch ? pr->toBranch->head : repo->commits;

  char mergeMsg[MAX_COMMIT_MSG_LEN];
  snprintf(mergeMsg, MAX_COMMIT_MSG_LEN, "%s (#%d)", pr->title, pr->id);

  Commit *squashCommit = new Commit;
  squashCommit->author = pr->author;
  strcpy(squashCommit->message, mergeMsg);
  squashCommit->timestamp = timestamp;
  squashCommit->prev = nullptr;
  squashCommit->next = targetHead;

  SHA1 hasher;
  initialize(hasher);
  input(hasher, squashCommit->author->name, strlen(squashCommit->author->name));
  input(hasher, squashCommit->message, strlen(squashCommit->message));
  input(hasher, squashCommit->timestamp);
  if (targetHead)
  {
    input(hasher, targetHead->author->name, strlen(targetHead->author->name));
    input(hasher, targetHead->message, strlen(targetHead->message));
    input(hasher, targetHead->timestamp);
  }
  digest(hasher);
  squashCommit->hash = hasher;

  if (targetHead)
    targetHead->prev = squashCommit;
  if (pr->toBranch)
    pr->toBranch->head = squashCommit;
  else
    repo->commits = squashCommit;

  Commit *oldTargetHead = targetHead;
  Commit *cur = pr->fromBranch->head;
  while (cur)
  {
    Commit *next = cur->next;
    bool isShared = false;
    for (Commit *m = oldTargetHead; m; m = m->next)
    {
      if (hashes_equal(cur->hash, m->hash))
      {
        isShared = true;
        break;
      }
    }
    if (isShared)
      break;
    delete cur;
    cur = next;
  }
  pr->fromBranch->head = nullptr;

  if (strcmp(pr->fromBranch->name, "main") != 0)
  {
    int newSize = repo->numBranches - 1;
    Branch **newBranches = new Branch *[newSize];
    int idx = 0;
    for (int i = 0; i < repo->numBranches; ++i)
    {
      if (repo->branches[i] != pr->fromBranch)
        newBranches[idx++] = repo->branches[i];
    }
    delete[] repo->branches;
    repo->branches = newBranches;
    repo->numBranches = newSize;
    delete pr->fromBranch;
  }

  pr->status = MERGED;
  cout << "Pull request #" << pr->id << " in " << repoFQN
       << " has been merged using squash merge." << endl;
}

/**
 * Task 8.2 - Merge Pull Request (Rebase Merge)
 *
 * The `merge_pull_request_rebasemerge` function merges the specified
 * pull request in a repsitory using the rebase merge strategy
 * (rebases all commits in the pull request to the target branch).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_rebasemerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp)
{
  // TODO: Task 8.2
  Repository *repo = nullptr;
  char temp[256];
  strcpy(temp, repoFQN);
  char *ownerName = strtok(temp, "/");
  char *repoName = strtok(nullptr, "/");
  if (!ownerName || !repoName)
    return;

  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, ownerName) == 0 &&
        strcmp(repoManagement.repos[i]->name, repoName) == 0)
    {
      repo = repoManagement.repos[i];
      break;
    }
  }
  if (!repo || prNumber <= 0 || prNumber > repo->numPrs)
  {
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }

  PullRequest *pr = repo->prs[prNumber - 1];
  if (pr->status != OPEN)
    return;

  Commit *targetHead = pr->toBranch ? pr->toBranch->head : repo->commits;
  if (targetHead == nullptr)
    targetHead = repo->commits;

  Commit *srcCommits[1024];
  int srcCount = 0;
  Commit *cur = pr->fromBranch->head;
  while (cur)
  {
    bool exists = false;
    for (Commit *tc = targetHead; tc; tc = tc->next)
    {
      if (hashes_equal(tc->hash, cur->hash))
      {
        exists = true;
        break;
      }
    }
    if (exists)
      break;
    srcCommits[srcCount++] = cur;
    cur = cur->next;
  }

  if (srcCount == 0)
  {
    for (int i = 0; i < repo->numBranches; ++i)
    {
      if (repo->branches[i] == pr->fromBranch)
      {
        for (int j = i; j < repo->numBranches - 1; ++j)
          repo->branches[j] = repo->branches[j + 1];
        repo->numBranches--;
        delete pr->fromBranch;
        break;
      }
    }
    pr->status = MERGED;
    cout << "Pull request #" << prNumber << " in " << repoFQN
         << " has been merged using rebase merge." << endl;
    return;
  }

  Commit *newCommits[1024];
  for (int i = 0; i < srcCount; ++i)
  {
    newCommits[i] = new Commit;
    newCommits[i]->author = srcCommits[i]->author;
    strcpy(newCommits[i]->message, srcCommits[i]->message);
    newCommits[i]->timestamp = timestamp;
  }

  for (int i = 0; i < srcCount; ++i)
  {
    Commit *c = newCommits[i];
    Commit *prevCommit = (i == 0) ? targetHead : newCommits[i - 1];

    SHA1 hasher;
    initialize(hasher);
    input(hasher, c->author->name, strlen(c->author->name));
    input(hasher, c->message, strlen(c->message));
    input(hasher, c->timestamp);
    if (prevCommit)
    {
      input(hasher, prevCommit->author->name, strlen(prevCommit->author->name));
      input(hasher, prevCommit->message, strlen(prevCommit->message));
      input(hasher, prevCommit->timestamp);
    }
    digest(hasher);
    c->hash = hasher;

    c->prev = (i == 0) ? nullptr : newCommits[i - 1];
    c->next = (i == srcCount - 1) ? targetHead : newCommits[i + 1];
    if (i > 0)
      newCommits[i - 1]->next = c;
    if (i == srcCount - 1 && targetHead)
    {
      targetHead->prev = c;
    }
  }

  Commit *newChainHead = newCommits[srcCount - 1];

  if (pr->toBranch)
    pr->toBranch->head = newChainHead;
  else
    repo->commits = newChainHead;
  if (pr->toBranch && strcmp(pr->toBranch->name, "main") == 0)
    repo->commits = newChainHead;

  for (int i = 0; i < srcCount; ++i)
    delete srcCommits[i];

  for (int i = 0; i < repo->numBranches; ++i)
  {
    if (repo->branches[i] == pr->fromBranch)
    {
      for (int j = i; j < repo->numBranches - 1; ++j)
        repo->branches[j] = repo->branches[j + 1];
      repo->numBranches--;
      delete pr->fromBranch;
      break;
    }
  }

  pr->status = MERGED;
  cout << "Pull request #" << prNumber << " in " << repoFQN
       << " has been merged using rebase merge." << endl;
}

/**
 * Task 8.3 - Merge Pull Request (Merge Commit)
 *
 * The `merge_pull_request_mergecommit` function merges the specified
 * pull request in a repsitory using the merge commit strategy
 * (adds all commits to the target branch preserving chronological order, with a
 * final merge commit added).
 *
 * This is slightly different from what actually happens when a merge commit
 * is used, but for simplicity's sake this is done instead.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_mergecommit(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp)
{
  // TODO: Task 8.3
  char fqnCopy[256];
  strcpy(fqnCopy, repoFQN);
  char *ownerName = strtok(fqnCopy, "/");
  char *repoNameFromFQN = strtok(nullptr, "/");
  if (!ownerName || !repoNameFromFQN)
    return;

  Repository *repo = nullptr;
  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    if (strcmp(repoManagement.repos[i]->owner->name, ownerName) == 0 &&
        strcmp(repoManagement.repos[i]->name, repoNameFromFQN) == 0)
    {
      repo = repoManagement.repos[i];
      break;
    }
  }
  if (!repo)
  {
    cout << "Repository " << repoFQN << " does not exist." << endl;
    return;
  }

  if (prNumber <= 0 || prNumber > repo->numPrs)
  {
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }

  PullRequest *pr = repo->prs[prNumber - 1];
  if (pr->status != OPEN)
    return;

  Commit *targetHead = pr->toBranch ? pr->toBranch->head : repo->commits;
  if (targetHead == nullptr)
    targetHead = repo->commits;

  Commit *toMove[1024];
  int moveCount = 0;
  Commit *cur = pr->fromBranch->head;
  while (cur)
  {
    bool exists = false;
    for (Commit *tc = targetHead; tc; tc = tc->next)
    {
      if (hashes_equal(tc->hash, cur->hash))
      {
        exists = true;
        break;
      }
    }
    if (exists)
      break;
    toMove[moveCount++] = cur;
    cur = cur->next;
  }

  if (moveCount > 0)
  {
    Commit *newHead = nullptr;
    for (int i = moveCount - 1; i >= 0; --i)
    {
      Commit *c = toMove[i];
      c->prev = nullptr;
      c->next = (i == moveCount - 1) ? targetHead : toMove[i + 1];
      if (i > 0)
        toMove[i - 1]->next = c;
      newHead = c;
    }
    if (pr->toBranch)
      pr->toBranch->head = newHead;
    else
      repo->commits = newHead;
    for (int i = 0; i < moveCount; ++i)
    {
      toMove[i]->prev = (i == 0) ? nullptr : toMove[i - 1];
      if (i == moveCount - 1 && targetHead)
        targetHead->prev = toMove[i];
    }
  }

  char fqBranch[256];
  snprintf(fqBranch, sizeof(fqBranch), "%s/%s:%s",
           pr->fromBranch->repo->owner->name,
           pr->fromBranch->repo->name,
           pr->fromBranch->name);

  char mergeMsg[MAX_COMMIT_MSG_LEN];
  snprintf(mergeMsg, MAX_COMMIT_MSG_LEN,
           "Merge pull request #%d from branch %s", pr->id, fqBranch);

  Commit *currentHead = pr->toBranch ? pr->toBranch->head : repo->commits;

  Commit *mergeCommit = new Commit;
  mergeCommit->author = pr->author;
  strcpy(mergeCommit->message, mergeMsg);
  mergeCommit->timestamp = timestamp;
  mergeCommit->prev = nullptr;
  mergeCommit->next = currentHead;

  SHA1 hasher;
  initialize(hasher);
  input(hasher, mergeCommit->author->name, strlen(mergeCommit->author->name));
  input(hasher, mergeCommit->message, strlen(mergeCommit->message));
  input(hasher, mergeCommit->timestamp);
  if (currentHead)
  {
    input(hasher, currentHead->author->name, strlen(currentHead->author->name));
    input(hasher, currentHead->message, strlen(currentHead->message));
    input(hasher, currentHead->timestamp);
  }
  digest(hasher);
  mergeCommit->hash = hasher;

  if (currentHead)
    currentHead->prev = mergeCommit;
  if (pr->toBranch)
    pr->toBranch->head = mergeCommit;
  else
    repo->commits = mergeCommit;

  if (strcmp(pr->fromBranch->name, "main") != 0)
  {
    int newSize = repo->numBranches - 1;
    Branch **newBranches = new Branch *[newSize];
    int idx = 0;
    for (int i = 0; i < repo->numBranches; ++i)
    {
      if (repo->branches[i] != pr->fromBranch)
        newBranches[idx++] = repo->branches[i];
    }
    delete[] repo->branches;
    repo->branches = newBranches;
    repo->numBranches = newSize;
    delete pr->fromBranch;
  }

  pr->status = MERGED;
  cout << "Pull request #" << pr->id << " in " << repoFQN
       << " has been merged using a merge commit." << endl;
}

/**
 * Task 9 - De-register User
 *
 * The `deregister_user` function de-registers a user from the version control
 * system and updates any references to it with the ghost user. This deletes all
 * their repositories and then deallocates memory allocated to this user.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositopries.
 * @param username: the name of the user to de-register.
 */
void deregister_user(UserManagement &userManagement,
                     RepositoryManagement &repoManagement,
                     const char *username)
{
  // TODO: Task 9
  User *toDel = nullptr;
  User *prev = nullptr;
  User *curr = userManagement.head;
  while (curr)
  {
    if (strcmp(curr->name, username) == 0)
    {
      toDel = curr;
      break;
    }
    prev = curr;
    curr = curr->next;
  }
  if (!toDel)
  {
    cout << "User " << username << " does not exist." << endl;
    return;
  }

  extern User *ghost;
  if (!ghost)
    return;

  for (int i = 0; i < repoManagement.numRepos; ++i)
  {
    Repository *repo = repoManagement.repos[i];

    Commit *c = repo->commits;
    while (c)
    {
      if (c->author == toDel)
        c->author = ghost;
      c = c->next;
    }

    for (int j = 0; j < repo->numPrs; ++j)
    {
      if (repo->prs[j]->author == toDel)
        repo->prs[j]->author = ghost;
    }

    for (int j = 0; j < repo->numBranches; ++j)
    {
      if (repo->branches[j]->creator == toDel)
        repo->branches[j]->creator = ghost;
    }
  }

  int i = 0;
  while (i < repoManagement.numRepos)
  {
    if (repoManagement.repos[i]->owner == toDel)
    {
      for (int j = i; j < repoManagement.numRepos - 1; ++j)
      {
        repoManagement.repos[j] = repoManagement.repos[j + 1];
      }
      repoManagement.numRepos--;
    }
    else
    {
      i++;
    }
  }

  if (prev)
  {
    prev->next = toDel->next;
  }
  else
  {
    userManagement.head = toDel->next;
  }

  delete[] toDel->repos;
  delete toDel;

  cout << "User " << username << " has been deregistered." << endl;
}