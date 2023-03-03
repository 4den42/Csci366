# CSCI 366 - Spring 2023

This is the root folder for the CSCI 366 class project.

You can find the following content in it:

* `/assembly` - assembly homework and tools
* `/c` - C homework and examples
* `/java` - Java homework and examples
* `/lmsm` - The class project, the Little Man Stack Machine
* `/grading` - The grading folder, where the autograder will push your grades up to

## Getting Your Private Copy

Please use the following steps to create a *private* copy of this repo for your work:

- Create a *private* repository in your own account by
    - Going to <https://github.com/new>
    - Enter the name `csci-366-spring2023-private`
    - Select `Private`
    - Navigate to the `Settings` -> `Manage Access` section
    - Add `1cg` as a collaborator

Once your repository is initialized, you can pull it down to your local machine:

```bash
$ git clone <your github repo url>
```

You can find the github repo url when you look at the repository in github.

Next, you should add the class repository as an upstream git repo:

```bash
$ cd csci-366-spring2023-private
$ git remote add upstream https://github.com/msu/csci-366-spring2023.git
$ git pull upstream master
$ git push origin master
```
This will synchronize your private repository with the class repository.

When you want to get an update from the public class repository you can run this command:

```
$ git pull upstream master
``` 

asdf
