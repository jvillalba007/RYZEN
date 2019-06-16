# tp-2019-1c-RYZEN
# Pasos a seguir antes de ponerse a laburar

1. `git fetch` para actualizar todo mi local y tener una copia por si se va todo a la mierda.
2. `git checkout mi-branch` para dirigirme a mi branch.
3. `git pull origin mi-branch` para colocar los cambios que estan en Github en mi branch.
4. Resolver conflictos si los hay, es decir, git te dice:
```
CONFLICT (content): Merge conflict in LFS/src/API.c
Automatic merge failed; fix conflicts and then commit the result.
```
Esto se hace yendo al código y viendo aquellas partes que tienen una separación con `===============`. La parte de arriba de ese separador es lo que vos tenes y la parte de abajo hasta el otro separador es lo que quiere cambiar lo que acabas de pullear. Al modificar eso, seguis.

5. `git add los-archivos-que-quiero-commitear`
6. `git status` para ver si los archivos que quiero commitear están en **verde**.
7. `git diff HEAD` para ver que lo que estoy añadiendo en verde y lo que estoy sacando en rojo es lo correcto.
8. `git commit -m 'mensaje que quiero meter'`.
9. `git push origin HEAD`.

### **NUNCA USAR** `-f`, `--force`, `git push` pelado. Si tenés algún conflicto, lee lo que dice git y googlealo y tratá de entender qué hiciste o consultalo con el grupo.

# Pasos a seguir para hacer un rebase

Un `rebase` es básicamente aplicar los nuevos commits de la branch **de la que yo partí** para hacer mi branch. Es decir, si de `master` saco una branch `mi-feature` y `master` le siguen haciendo cambios, mi branch `mi-feature` se "quedó vieja" con respecto a `master` y tengo que actualizarla, i.e `rebase`earla, i.e adelantar el commit de donde parte mi branch.

1. `git fetch`
2. `git checkout la-branch-que-se-actualizo`
3. `git pull origin la-branch-que-se-actualizo`
4. `git checkout mi-feature`
5. `git rebase la-branch-que-se-actualizo`

Listo! Podes seguír trabajando y pusheando normalmente. Si git no le gusta cuando pusheas después, corroborá que tu branch este bien igual que Github haciendo `git log` y fijándose en Github los commits de tu branch a ver si coinciden. Si esta todo bien `git push origin HEAD --force-with-lease`. AHORA CORROBORÁ QUE LO QUE ESTÁ EN GITHUB EN TU BRANCH ES CORRECTO.
