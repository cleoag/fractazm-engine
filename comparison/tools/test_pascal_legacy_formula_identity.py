"""Only the three source-defined, forward legacy Flip migrations are allowed."""
import unittest

from pascal_formula_identity import validate_formula_load


class LegacyFormulaIdentityTests(unittest.TestCase):
    scene = r'C:\scenes\legacy-flip.m3p'

    def check(self, saved, loaded, number=20, count=1):
        log = ('[MB3D] Loading parameters: '+self.scene+'\n'
               f'[MB3D] Formula[0]: iFnr={number} iItCount={count} name={loaded}\n')
        return validate_formula_load(log,self.scene,
            [dict(slot=0,number=20,iterations=1,name=saved)])

    def test_forward_migration_is_reported_and_scoped(self):
        for axis in ('XY','XZ','YZ'):
            with self.subTest(axis=axis):
                result=self.check('_Flip'+axis,'_Flip'+axis+'c')
                migration=result['file_load_migrations'][0]
                self.assertEqual(migration['saved_name'],'_Flip'+axis)
                self.assertEqual(migration['loaded_name'],'_Flip'+axis+'c')
                self.assertEqual(migration['option_0'],0)
                self.assertIn('not direct',migration['scope'])

    def test_wrong_axis_and_reverse_alias_remain_errors(self):
        for saved,loaded in (('_FlipXY','_FlipXZc'),('_FlipXYc','_FlipXY'),
                             ('Other','_FlipXYc'),('_FlipXY','Other')):
            with self.subTest(saved=saved,loaded=loaded):
                with self.assertRaisesRegex(ValueError,'load mismatch'):
                    self.check(saved,loaded)

    def test_migration_does_not_accept_disabled_formula(self):
        for number,count in ((-1,0),(20,0),(0,1)):
            with self.subTest(number=number,count=count):
                with self.assertRaisesRegex(ValueError,'load mismatch'):
                    self.check('_FlipXY','_FlipXYc',number,count)


if __name__=='__main__': unittest.main()
